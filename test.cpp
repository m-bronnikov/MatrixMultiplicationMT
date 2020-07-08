// Made by Max Bronnikov
#include <string>
#include <iostream>
#include <vector>
#include "myself/QuadratMatrix.h"

using namespace std;

#define NUM_OF_THREADS 3

/* ТЕСТ ПРОВЕРЯЕТ ПРАВИЛЬНОСТЬ РЕЗУЛЬТАТОВ СО СГЕНЕРИРОВАННЫМИ В PYTHON */

int main(){
    // перед использованием требуется инициализация потоков
    myself::QuadratMatrix::create_threads(NUM_OF_THREADS);

    // пути к матрицам
    string left_path = "tests/leftN.mtx";
    string right_path = "tests/rightN.mtx";
    string ans_path = "temp/answerN.mtx";
    string res_path = "tests/resultN.mtx";

    // общий результат тестирования
    bool final = true;

    // для удобства оставим пока 9 тестов
    cout << "Testing:" << endl;
    for(char c = '1'; c <= '9'; ++c){
        // меняем путь к тесту изменив один символ
        left_path[10] = c;
        right_path[11] = c;
        ans_path[11] = c;
        res_path[12] = c;
        // создаем матрицы
        myself::QuadratMatrix left(left_path);
        myself::QuadratMatrix right(right_path);
        myself::QuadratMatrix res(res_path);
        myself::QuadratMatrix ans = left * right;
        // сравнение с результатом
        bool compare = (res == ans);
        cout << "Test №" << c << ": " << (compare ? "TRUE" : "FALSE") << endl; 

        final &= compare;

        // запись ответа в файл
        ans.to_file(ans_path);
    }

    // итог тестирования
    cout << "All tests correct: " << (final ? "TRUE" : "FALSE") << endl;

    return 0;
}

