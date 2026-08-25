#pragma once

#include "clientconfig.h"


namespace will {


class ClientConfigParser {
public:
    ClientConfigParser(int argc, char* argv[]);

    const ClientConfig& client_config() const noexcept;

private:
    ClientConfig client_config_;
};


} // namespace will
