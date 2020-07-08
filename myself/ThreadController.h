// Made by Max Bronnikov
#ifndef THREAD_CONTROLLER_H
#define THREAD_CONTROLLER_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

// Опишем Thread Pool, котрый будет выполнять за нас функкции в потоках без траты времени
// на создание и инициализации новых

namespace myself{

class ThreadController{
public:
    // конструктор
    ThreadController(size_t);
    // добавить функцию для асинхронного выполнения
    template<class F, class... Args>
    auto to_work(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
    // узнать количество потоков
    size_t workers();
    // деструктор
    ~ThreadController();

private:
    // потоки
    std::vector< std::thread > threads;
    // очередь из задач для выполнения
    std::queue< std::function<void()> > tasks;
    // количество потоков
    size_t num_threads;
    // для синхронизации работы класса
    std::mutex queue_mutex;
    std::condition_variable condition;
    // флаг остановки работы класса (выбрасывается деструктором)
    bool stop;
};


// В конструкторе создаётся указанное число потоков-демонов, ждущих елементов в очереди
inline ThreadController::ThreadController(size_t n_threads)
: num_threads(n_threads), stop(false){
    // создаём указанное количество потоков
    for(size_t i = 0; i < n_threads; ++i){
        // emplace_back позволяет передавать не сам объект(thread),
        // а аргументы для конструктора, что быстрее и практичнее
        threads.emplace_back(
            // эта лямбда будет выполняться во всех потоках, ожидая работу
            [this]{
                while(true){
                    // нас не волнуют аргументы функции, поскольку мы делаем bind
                    // при добавлении функции(см ниже в to_work)
                    std::function<void()> task;

                    {   
                        // condition variable требует unique_lock
                        std::unique_lock<std::mutex> guard(this->queue_mutex);
                        // ожидание сигнала о появлении новых задач или о завершении работы
                        this->condition.wait(guard,
                            // перегрузка от ложных пробуждений
                            [this]{ return this->stop || !this->tasks.empty(); });
                        // если закончились задачи и приказано остановиться
                        if(this->stop && this->tasks.empty())
                            // убивается поток
                            return;
                        // иначе берем задачу из очереди
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    // и выполняем
                    task();
                }
            }
        );
    }

}

// количество рабочих потоков
size_t ThreadController::workers(){
    return num_threads;
}

// Добавление задачи в рабочий пул
// будем возвращать future для получение результата вычисления по требованию
template<class F, class... Args>
auto ThreadController::to_work(F&& f, Args&&... args) 
-> std::future<typename std::result_of<F(Args...)>::type>{

    using return_type = typename std::result_of<F(Args...)>::type;

    // пакуем задачу из переданной функции, чтобы получить её future
    auto task = std::make_shared<std::packaged_task<return_type()>>(
            // бинд позволяет создать задачу, запуск которой в потоках 
            // не потребует переменных
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
    
    // непосредственно future, который вернет текущий метод
    std::future<return_type> res = task->get_future();

    // область для RAII работы с мьютексом
    // мьютекс призван защитить очередь tasks
    {
        std::unique_lock<std::mutex> guard(queue_mutex);
        // помещаем задачу в очередь
        tasks.emplace([task](){ (*task)(); });
    }
    // позволяем одному потоку приступить к задаче
    condition.notify_one();
    return res;
}

// деструктор
inline ThreadController::~ThreadController()
{   
    // флаг останова поднят
    {
        std::unique_lock<std::mutex> guard(queue_mutex);
        stop = true;
    }
    // пробуждаем спящих
    condition.notify_all();
    // дожидаемся окончания потоков
    for(std::thread &worker : threads)
        worker.join();
}


}

#endif
