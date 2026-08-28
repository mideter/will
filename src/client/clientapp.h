#pragma once

#include "clientconfig.h"
#include "clientcliapp.h"


namespace will {


class ClientApp {
public:
    ClientApp(int argc, char* argv[]);

    const ClientConfig& config() const noexcept;

    int run();

private:
    ClientCliApp cli_;
    ClientConfig config_;
};


} // namespace will
