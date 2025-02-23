#include <stdio.h>
#include <pthread.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* thread1_function(void* arg) {
    pthread_mutex_lock(&mutex1);
    printf("Thread 1: Acquired mutex1\n");

    // Добавим задержку для увеличения шансов на взаимную блокировку

    pthread_mutex_lock(&mutex2);
    printf("Thread 1: Acquired mutex2\n");

    // Делаем что-то с ресурсами, защищенными mutex1 и mutex2

    pthread_mutex_unlock(&mutex2);
    printf("Thread 1: Released mutex2\n");
    
    pthread_mutex_unlock(&mutex1);
    printf("Thread 1: Released mutex1\n");

    return NULL;
}

void* thread2_function(void* arg) {
    pthread_mutex_lock(&mutex2);
    printf("Thread 2: Acquired mutex2\n");

    // Добавим задержку для увеличения шансов на взаимную блокировку

    pthread_mutex_lock(&mutex1);
    printf("Thread 2: Acquired mutex1\n");

    // Делаем что-то с ресурсами, защищенными mutex1 и mutex2

    pthread_mutex_unlock(&mutex1);
    printf("Thread 2: Released mutex1\n");

    pthread_mutex_unlock(&mutex2);
    printf("Thread 2: Released mutex2\n");

    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    if (pthread_create(&thread1, NULL, thread1_function, NULL) != 0 ||
        pthread_create(&thread2, NULL, thread2_function, NULL) != 0) {
        perror("Error creating threads");
        return 1;
    }

    if (pthread_join(thread1, NULL) != 0 || pthread_join(thread2, NULL) != 0) {
        perror("Error joining threads");
        return 1;
    }

    printf("Both threads have finished.\n");

    return 0;
}