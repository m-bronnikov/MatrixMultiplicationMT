// Made by Max Bronnikov
#include <string>
#include <iostream>
#include <vector>
#include <ctime>
#include "myself/QuadratMatrix.h"

using namespace std;

/* ДЕМОНСТРАЦИЯ РАБОТЫ */


// в аргументах командной строки указывается количесвто потоков программы
int main(int argc, char *argv[]){
    // для случайных чисел
    srand(time(0));

    // по дефолту
    size_t threads = 4;

    // берем количество из командной строки
    if(argc == 2){
        threads = atoi(argv[1]);
    }else if(argc > 2){
        cout << "Wrong args for program! Exit!" << endl;
        return 1;
    }

    // так как один поток - текущий =>
    --threads;

    // пока не добавлена реализация последоваельного перемножения
    // не стоит заапускать программу всего с 1 потоком
    if(threads < 1){
        cout << "Sorry, but use more threads! Exit!" << endl;
        return 2;
    }

    // перед использованием требуется инициализация потоков
    myself::QuadratMatrix::create_threads(threads);

    // создаем 2 файла
    myself::QuadratMatrix left;
    myself::QuadratMatrix right;

    // и случайно их инициализируем
    for(size_t i = 0; i < left.get_size(); ++i){
        for(size_t j = 0; j < left.get_size(); ++j){
            left[i][j] = rand();
            right[i][j] = rand();
        }
    }
    
    string ans_path = "temp/example.mtx";

    // параллельное перемножение
    myself::QuadratMatrix ans = left * right;

    // запись ответа в файл
    ans.to_file(ans_path);

    return 0;
}

