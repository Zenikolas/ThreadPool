#include <iostream>
#include <vector>
#include <functional>
#include <future>
#include <queue>
#include <fstream>

namespace threadUtils
{

class ThreadPool
{
public:
    ThreadPool(size_t);

    template<class F, class ... Args>
    std::future<typename std::result_of<F(Args...)>::type> addTask(F&&, Args&& ...);

    ~ThreadPool();
private:
    void threadFunc();

    std::queue<std::function<void()>> m_tasks;
    std::vector<std::thread> m_workers;

    std::mutex m_mut;
    std::condition_variable m_cv;

    std::atomic<bool> m_stop;
};

ThreadPool::ThreadPool(size_t nth) : m_stop(false)
{
    m_workers.reserve(nth);

    for (size_t i = 0; i < nth; ++i) {
        m_workers.emplace_back([this] () { threadFunc(); });
    }
}

template<class F, class ... Args>
std::future<typename std::result_of<F(Args...)>::type> ThreadPool::addTask(F&& f, Args&&... args)
{
    using retType = typename std::result_of<F(Args ...)>::type;
    auto task = std::make_shared< std::packaged_task<retType()> >([f, args...] () { return f(args...); });
    auto ret = task->get_future();

    {
        std::unique_lock<std::mutex> lc(m_mut);
        if (m_stop) {
            throw std::runtime_error("Trying to add new task to stopped ThreadPool");
        }

        m_tasks.push([task] () mutable { (*task)(); });
    }

    m_cv.notify_one();

    return ret;
}

void ThreadPool::threadFunc()
{
    while (true) {
        std::function<void()> task;
        std::unique_lock<std::mutex> lc(m_mut);
        m_cv.wait(lc, [this](){return !m_tasks.empty() || m_stop.load(std::memory_order_acquire); });

        if (m_stop && m_tasks.empty()) {
            return;
        }

        task = std::move(m_tasks.front());
        m_tasks.pop();

        if (task) {
            task();
        }
    }
}

ThreadPool::~ThreadPool()
{
    m_stop.store(true, std::memory_order_release);
    m_cv.notify_all();

    for (auto& w : m_workers) {
        w.join();
    }
}

}


int main()
{
    constexpr size_t size = 3e5;
    std::vector< std::future<size_t> > results;
    results.reserve(size);

    auto func = [] (size_t n) {
        size_t ret = 0;
        for (size_t i = 0; i < n; ++i) {
            ret += i * n;
        }

        return ret;
    };

    auto start = std::chrono::high_resolution_clock::now();

    threadUtils::ThreadPool thPool(10);

    for (size_t i = 0; i < size; ++i) {
        results.emplace_back( thPool.addTask(func, i * 200) );
    }

    for (size_t i = 0; i < size; ++i) {
        results[i].wait();
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start);

    std::cout << "Elapse time: " << elapsed.count() << std::endl;
    return 0;
}