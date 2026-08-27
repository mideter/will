#pragma once

#include "clientconfig.h"


namespace will {


class ClientApp {
public:
    ClientApp(int argc, char* argv[]);

    const ClientConfig& config() const noexcept;

    int run();

private:
    ClientConfig config_;
};


} // namespace will
