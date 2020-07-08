# Made by Max Bronnikov
import numpy as np
import os
from tqdm import tqdm

MATRIX_SIZE = 100

def to_file(path, matrix):
    with open(path, 'w') as f:
        for i in range(matrix.shape[0]):
            for j in range(matrix.shape[1]):
                f.write(str(matrix[i][j]) + "\t")
            f.write("\n")


# Код генерирует 9 случайных матриц и вычисляет их произведение с записью в файл
print("Generating test:")
for i in tqdm(range(1, 10)):
    left_path = "tests/left" + str(i) + ".mtx"
    right_path = "tests/right" + str(i) + ".mtx"
    res_path = "tests/result" + str(i) + ".mtx"

    left = np.random.randint(-2000, 2000, (MATRIX_SIZE, MATRIX_SIZE))
    right = np.random.randint(-2000, 2000, (MATRIX_SIZE, MATRIX_SIZE))
    res = np.dot(left, right)

    to_file(left_path, left)
    to_file(right_path, right)
    to_file(res_path, res)

print("Test generated!")


