// Made by Max Bronnikov
#ifndef QUADRAT_MATRIX_H
#define QUADRAT_MATRIX_H

#include <vector>
#include <memory>
#include <fstream>
#include <thread>
#include "ThreadController.h"

namespace myself{

// размер матрицы можно менять
#define MATRIX_SIZE 100

// опишем матрицу с параллельным перемножением
class QuadratMatrix{
public:
    // нулевая матрица
    QuadratMatrix();
    // копирование
    QuadratMatrix(const QuadratMatrix& matrix);
    // перемещение
    QuadratMatrix(QuadratMatrix&& matrix);
    // из файла
    QuadratMatrix(std::string& path);

    // создание пула с указанным количеством потоков
    // стоит вызывать лишь раз
    static void create_threads(size_t n_treads);

    // параллельное перемножение матриц
    friend const QuadratMatrix 
    operator*(const QuadratMatrix& left, const QuadratMatrix& right);

    // доступ к элементам
    std::vector<double>& operator[](const size_t index);
    const std::vector<double>& operator[](const size_t index) const;

    // компаратор
    friend bool 
    operator==(const QuadratMatrix& right, const QuadratMatrix& left);

    // вывод в поток
    friend std::ostream& 
    operator<<(std::ostream& os, const QuadratMatrix& matrix);
    // ввод из потока
    friend std::istream& 
    operator>>(std::istream& is, QuadratMatrix& matrix);

    // размер матрицы 
    size_t get_size();
    // количество потоков
    size_t get_workers();

    // запись матрицы в файл 
    void to_file(std::string& path);
    // чтение матрицы из файла
    void from_file(std::string& path);

    // деструктор
    ~QuadratMatrix(){};

private:
    // controller
    static std::shared_ptr<ThreadController> daemons;
    // размер массива(для удобства, задается в define)
    size_t size;
    // сама матрица
    std::vector<std::vector<double>> data;

    // функция считает элемент A[i][j] результирующей матрицы
    friend void 
    compute_i_line(const std::vector<double>& l, const QuadratMatrix& r, std::vector<double>& res);
};


// изначально пустой указатель на пул 
std::shared_ptr<ThreadController> QuadratMatrix::daemons(nullptr);

QuadratMatrix::QuadratMatrix(){
    // задаем размер
    size = MATRIX_SIZE;
    // выделяем память и инициализируем нулями
    data.assign(size, std::vector<double>(size, 0.0));
}

QuadratMatrix::QuadratMatrix(const QuadratMatrix& matrix){
    size = matrix.size;
    // копируем даннные
    data = matrix.data;
}

QuadratMatrix::QuadratMatrix(QuadratMatrix&& matrix){
    size = matrix.size;
    // перемещаем данные
    data = std::move(matrix.data);
    // думаю, матрица должна оставаться размера MATRIX_SIZE
    matrix.data.assign(size, std::vector<double>(size, 0.0));
}

// конструктор открывает указанный файл и читает матрицу
QuadratMatrix::QuadratMatrix(std::string& path)
: QuadratMatrix(){
    from_file(path);
}

void QuadratMatrix::from_file(std::string& path){
    std::ifstream file(path, std::ios::in);
    // здесь стоит добавить проверку на успех
    // и выбросить искл в случае неудачи
    file >> *this;
    file.close();
}

void QuadratMatrix::to_file(std::string& path){
    std::ofstream file(path, std::ios::out);
    // здесь стоит добавить проверку на успех
    // и выбросить искл в случае неудачи
    file << *this;
    file.close();
}

std::ostream& operator<<(std::ostream& os, const QuadratMatrix& matrix){
    for(size_t i = 0; i < matrix.size; ++i){
        for(size_t j = 0; j < matrix.size; ++j){
            os << matrix.data[i][j] << '\t';
        }
        os << std::endl;
    }
    return os;
}

std::istream& operator>>(std::istream& is, QuadratMatrix& matrix){
    for(size_t i = 0; i < matrix.size; ++i){
        for(size_t j = 0; j < matrix.size; ++j){
            is >> matrix.data[i][j];
        }
    }
    return is;
}

size_t QuadratMatrix::get_size(){
    return size;
}

size_t QuadratMatrix::get_workers(){
    return daemons->workers();
}

std::vector<double>& QuadratMatrix::operator[](const size_t index){
    return data[index];
}

bool operator==(const QuadratMatrix& r, const QuadratMatrix& l){
    for(size_t i = 0; i < r.size; ++i){
        for(size_t j = 0; j < r.size; ++j){
            if(r[i][j] != l[i][j]){
                return false;
            }
        }
    }
    return true;
}

// константная версия
const std::vector<double>& QuadratMatrix::operator[](const size_t index) const{
    return data[index];
}


void QuadratMatrix::create_threads(size_t n_threads){
    daemons = std::make_shared<ThreadController>(n_threads);
}

// функция в отдельном потоке вычисляет результирующую строку матрицы, переданной последним параметром
// первый элемент - сответствующая строка левой матрицы
void compute_i_line(const std::vector<double>& l, const QuadratMatrix& r, std::vector<double>& res){
    // для каждого элемента строки
    for(size_t j = 0; j < r.size; ++j){
        double ans = 0.0;
        for(size_t k = 0; k < r.size; ++k){
            ans += l[k] * r[k][j];
        }
        res[j] = ans;
    }
}

// параллельное перемножение матриц
// в будущем следует добавить просто последоваельную реализацию 
// и соответствующий флаг для переключения между ними
const QuadratMatrix operator*(const QuadratMatrix& l, const QuadratMatrix& r){
    QuadratMatrix ans;
    // в отдельных потоках будем вычислять жлементы результирующей матрицы
    // по потоку на строку
    // для этого для каждой из строк будем хранить 
    // по future(возможно в будущем стоит экономнее расходовать память)
    std::vector<std::future<void>> futures(ans.size);
    // создаем задачи
    for(size_t i = 0; i < ans.size; ++i){
        // в очередь на выполнение
        futures[i] = 
            // для передачи параметров в поток следует использовать std::ref
            QuadratMatrix::daemons->to_work(compute_i_line, std::ref(l[i]), std::ref(r), std::ref(ans[i]));
    }

    // ожидаем результаты
    for(size_t i = 0; i < ans.size; ++i){
        futures[i].get();
    }
    return ans;
}

}

#endif