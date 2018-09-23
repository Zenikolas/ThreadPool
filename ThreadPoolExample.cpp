#include <iostream>
#include "ThreadPool.hpp"


int main()
{
    constexpr size_t size = 16e3;
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