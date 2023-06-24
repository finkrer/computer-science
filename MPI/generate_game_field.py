import numpy as np

rows = 10000
cols = 10000

field = np.random.randint(2, size=(rows, cols))

with open("input.txt", "w") as f:
    f.write(f"{rows} {cols}\n")
    for row in field:
        f.write(" ".join(map(str, row)) + "\n")
