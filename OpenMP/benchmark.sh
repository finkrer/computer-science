#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: ./benchmark.sh <filename>"
    exit 1
fi

file=$1

for thread in {1..16}; do
    echo "$thread threads:"
    export OMP_NUM_THREADS=$thread
    /usr/bin/time ./OpenMP $file > /dev/null
done


