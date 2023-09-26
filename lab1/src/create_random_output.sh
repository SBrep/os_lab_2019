#!/bin/bash

exec 3> numbers.txt

for ((i = 0; i < 150; i++))
do
    output=$(od -An -N1 -t dI /dev/random)
    echo $output >&3
done

exec 3>&-   #закрытие дескриптора