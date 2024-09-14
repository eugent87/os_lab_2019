#!/bin/bash
count=$#
sum=0
for arg in "$@"
do
    sum=$((sum + arg))
done
average=$((sum / count))
echo "Count: $count"
echo "Average: $average"