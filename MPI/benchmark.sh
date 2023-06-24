#!/bin/bash

for count in {1..8}; do
    echo "$count processes:"
    /usr/bin/time  mpirun -np $count ./MPI
done

