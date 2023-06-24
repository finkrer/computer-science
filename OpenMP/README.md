clang 16.0.6

Apple M1 (arm64, 8 cores)

test data:

```bash
python3 generate_test_file.py 50000 test.txt
```

compiled with:

```bash
/opt/homebrew/opt/llvm/bin/clang++ -fopenmp main.cpp -o OpenMP
```

Задача хорошо масштабируется на реальные 8 потоков, которые есть в системе. 9-11 потоков также работают неплохо. 12+ потоков уже много, время выполнения быстро увеличивается. Возможно, это вызвано постоянным переключением контекста и промахами в кэше.

```
1 threads:
       38.57 real        36.87 user         0.25 sys
2 threads:
       27.73 real        36.61 user         0.22 sys
3 threads:
       22.36 real        40.83 user         0.25 sys
4 threads:
       18.16 real        42.94 user         0.15 sys
5 threads:
       16.75 real        48.11 user         0.35 sys
6 threads:
       16.54 real        54.35 user         0.55 sys
7 threads:
       14.20 real        54.90 user         0.40 sys
8 threads:
       13.30 real        55.43 user         0.42 sys
9 threads:
       14.47 real        63.02 user         0.68 sys
10 threads:
       12.19 real        65.25 user         0.43 sys
11 threads:
       12.78 real        73.98 user         0.51 sys
12 threads:
       30.23 real       181.00 user         1.59 sys
13 threads:
       49.99 real       305.22 user         2.81 sys
14 threads:
      152.37 real      1010.32 user         7.52 sys
15 threads:
      155.99 real      1009.76 user         7.64 sys
16 threads:
      282.12 real      1951.42 user        12.11 sys
```
