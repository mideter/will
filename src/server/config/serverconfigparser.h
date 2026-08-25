#pragma once

#include "serverconfig.h"


namespace will {


class ServerConfigParser {
public:
    ServerConfigParser(int argc, char* argv[]);

    const ServerConfig& server_config() const noexcept;

private:
    ServerConfig server_config_;
};


} // namespace will
