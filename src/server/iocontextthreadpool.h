#pragma once

#include <asio.hpp>

#include <thread>
#include <vector>


namespace will {


/** Runs io_context::run() on a pool of jthreads; joins on destruction. */
class IoContextThreadPool {
public:
    IoContextThreadPool(asio::io_context& ioc, int thread_count);

    IoContextThreadPool(const IoContextThreadPool&) = delete;
    IoContextThreadPool& operator=(const IoContextThreadPool&) = delete;

private:
    void run_worker();

    asio::io_context& ioc_;
    std::vector<std::jthread> threads_;
};


} // namespace will
