#include "revert_string.h"

void RevertString(char *str)
{
	char* start_of_str = str;	//переменная хранит адрес первогот элемента строки

	int length_of_str = 0;		//длина строки
	if (str == 0){				//если указатель нулевой, то возврат из функции
		return;
	}

	//цикл для поиска длины строки
	while (*str != '\0'){
		str++;
		length_of_str++;
	}

	//возврат указателя к началу данных нашей строки
	str = start_of_str;

	//цикл для реверса строки
	for (int i = 0, j = length_of_str - 1; i < j; i++, j--)
    {
        char ch = str[i];
        str[i] = str[j];
        str[j] = ch;
    }
}