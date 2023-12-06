#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include "common/common.h"

// Функция для вычисления факториала в заданном диапазоне
uint64_t Factorial(const struct FactorialArgs *args) {
    uint64_t ans = 1;

    // Вычисление факториала для каждого значения в заданном диапазоне
    for (uint64_t i = args->begin; i <= args->end; i++) {
        ans = MultModulo(ans, i, args->mod);
    }

    return ans;
}

// Функция, являющаяся точкой входа для потока
void *ThreadFactorial(void *args_ptr) {
    // Приведение указателя на аргументы к соответствующему типу
    struct FactorialArgs *args = (struct FactorialArgs *)args_ptr;

    // Вычисление факториала для заданного диапазона
    uint64_t result = Factorial(args);

    // Возврат результата как указателя void (требование для pthread_join)
    return (void *)result;
}

int main(int argc, char **argv) {
    // Инициализация переменных
    int tnum = -1;  // Количество потоков
    int port = -1;  // Порт, на котором слушает сервер

    // Обработка аргументов командной строки с использованием getopt_long
    while (true) {
        static struct option options[] = {
                {"port", required_argument, 0, 0},
                {"tnum", required_argument, 0, 0},
                {0, 0, 0, 0},
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 0: {
                switch (option_index) {
                    case 0:
                        port = atoi(optarg);
                        break;
                    case 1:
                        tnum = atoi(optarg);
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
            } break;

            case '?':
                printf("Unknown argument\n");
                break;
            default:
                fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    // Проверка наличия обязательных параметров
    if (port == -1 || tnum == -1) {
        fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
        return 1;
    }

    // Создание сокета
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        fprintf(stderr, "Can not create server socket!");
        return 1;
    }

    // Настройка структуры sockaddr_in для сервера
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons((uint16_t)port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // Установка параметра SO_REUSEADDR для повторного использования адреса
    int opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

    // Привязка сокета к адресу и порту
    int err = bind(server_fd, (struct sockaddr *)&server, sizeof(server));
    if (err < 0) {
        fprintf(stderr, "Can not bind to socket!");
        return 1;
    }

    // Перевод сокета в режим прослушивания
    err = listen(server_fd, 128);
    if (err < 0) {
        fprintf(stderr, "Could not listen on socket\n");
        return 1;
    }

    // Вывод информации о том, что сервер слушает на порту
    printf("Server listening at %d\n", port);

    // Бесконечный цикл для обработки подключений от клиентов
    while (true) {
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);

        // Принятие нового подключения от клиента
        int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

        // Обработка ошибки при подключении клиента
        if (client_fd < 0) {
            fprintf(stderr, "Could not establish new connection\n");
            continue;
        }

        // Бесконечный цикл для обработки запросов от клиента
        while (true) {
            struct FactorialArgs args;

            // Получение задания от клиента
            int read = recv(client_fd, &args, sizeof(args), 0);

            uint64_t total_end = args.end;

            // Проверка на конец потока данных от клиента
            if (!read)
                break;
            if (read < 0) {
                fprintf(stderr, "Client read failed\n");
                break;
            }
            if (read < sizeof(args)) {
                fprintf(stderr, "Client sent wrong data format\n");
                break;
            }

            // Создание массива потоков для выполнения вычислений
            pthread_t threads[tnum];
            uint64_t chunk_size = (args.end - args.begin + 1) / tnum;

            uint64_t start = args.begin;
            uint64_t end = start + chunk_size - 1;

            uint64_t total = 1;

            // Запуск потоков для выполнения вычислений в различных частях диапазона
            for (int i = 0; i < tnum; i++) {
                if (i == tnum - 1) {
                    end = total_end;
                }

                // Выделение памяти для структуры аргументов потока
                struct FactorialArgs *thread_args = malloc(sizeof(struct FactorialArgs));
                if (thread_args == NULL) {
                    fprintf(stderr, "Error: failed to allocate memory for thread args\n");
                    return 1;
                }

                // Заполнение аргументов потока
                thread_args->begin = start;
                thread_args->end = end;
                thread_args->mod = args.mod;

                // Создание потока
                if (pthread_create(&threads[i], NULL, ThreadFactorial, thread_args) != 0) {
                    fprintf(stderr, "Error: pthread_create failed!\n");
                    return 1;
                }

                start = end + 1;
                end = start + chunk_size - 1;
            }

            // Дожидаемся завершения всех потоков
            for (int i = 0; i < tnum; i++) {
                uint64_t result = 0;
                pthread_join(threads[i], (void **)&result);
                total = MultModulo(total, result, args.mod);
            }

            // Вывод общего результата
            printf("Total: %lu\n", total);

            // Отправка результата клиенту
            err = send(client_fd, &total, sizeof(total), 0);
            if (err < 0) {
                fprintf(stderr, "Can't send data to client\n");
                break;
            }
        }

        // Закрытие сокета клиента
        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
    }

    // Закрытие серверного сокета (код, вероятно, никогда не достигнет этого места)
    close(server_fd);

    return 0;
}