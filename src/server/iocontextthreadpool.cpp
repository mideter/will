#include "iocontextthreadpool.h"

#include <iostream>


namespace will {


IoContextThreadPool::IoContextThreadPool(asio::io_context& ioc, int thread_count)
    : ioc_(ioc)
{
    for (int i = 0; i < thread_count; ++i)
        threads_.emplace_back([this] { run_worker(); });
}


void IoContextThreadPool::run_worker()
try {
    ioc_.run();
}
catch (const std::exception& e) {
    std::cerr << "io_context worker error: " << e.what() << '\n';
}


} // namespace will
