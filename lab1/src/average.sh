#!/bin/bash

amount=$#

sum=0
for arg in "$@"
do
sum=$(($sum+$arg))
done

echo "amount of arguments = $amount"
echo "average of arguments = $(($sum/$amount))"