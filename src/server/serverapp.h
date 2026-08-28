#pragma once

#include "serverconfig.h"
#include "servercliapp.h"


namespace will {


class ServerApp {
public:
    ServerApp(int argc, char* argv[]);

    const ServerConfig& config() const noexcept;

    int run();

private:
    ServerCliApp cli_;
    ServerConfig config_;
};


} // namespace will
