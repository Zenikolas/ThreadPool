#include <iostream>
#include "ThreadPool.hpp"

using namespace std::chrono;

size_t func(size_t n) {
    volatile size_t ret = 0;
    for (size_t i = 0; i < 1'000'000; ++i) {
        ret += i * n + 1;
    }

    return ret;
}

template<class Func, class ... Args>
milliseconds mesureFunc(Func&& func, Args&& ... args) {
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    std::forward<Func>(func)(std::forward<Args>(args)...);

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    return duration_cast<milliseconds>(t2 - t1);
}

void oneThreadTest(size_t size) {
    std::vector<size_t> results;
    results.reserve(size);

    for (size_t i = 0; i < size; ++i) {
        results.emplace_back(func(i));
    }
}

void threadPoolTest(size_t size, unsigned int threads) {
    std::vector<std::future<size_t> > results;
    results.reserve(size);
    threadUtils::ThreadPool thPool(threads);

    for (size_t i = 0; i < size; ++i) {
        results.emplace_back(thPool.addTask(func, i));
    }

    for (size_t i = 0; i < size; ++i) {
        results[i].wait();
    }
}


int main() {
    std::cout << "One thread 1000: " << mesureFunc(&oneThreadTest, 1'000).count() <<
              std::endl;
    std::cout << "One thread 5000: " << mesureFunc(&oneThreadTest, 5'000).count() <<
              std::endl;
    std::cout << "One thread 10000: " << mesureFunc(&oneThreadTest, 10'000).count() <<
              std::endl;

    std::cout << "ThreadPool, task size 1000, workers 2: "
              << mesureFunc(&threadPoolTest, 1'000, 2).count() << std::endl;
    std::cout << "ThreadPool, task size 5000, workers 2: "
              << mesureFunc(&threadPoolTest, 5'000, 2).count() << std::endl;
    std::cout << "ThreadPool, task size 10000, workers 2: "
              << mesureFunc(&threadPoolTest, 10'000, 2).count() << std::endl;

    std::cout << "ThreadPool, task size 1000, workers 4: "
              << mesureFunc(&threadPoolTest, 1'000, 4).count() << std::endl;
    std::cout << "ThreadPool, task size 5000, workers 4: "
              << mesureFunc(&threadPoolTest, 5'000, 4).count() << std::endl;
    std::cout << "ThreadPool, task size 10000, workers 4: "
              << mesureFunc(&threadPoolTest, 10'000, 4).count() << std::endl;
}