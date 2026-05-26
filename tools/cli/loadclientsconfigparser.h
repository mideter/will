#pragma once

#include "loadclientsconfig.h"


namespace will {


class LoadClientsConfigParser {
public:
    LoadClientsConfigParser(int argc, char* argv[]);

    const LoadClientsConfig& load_config() const noexcept;

private:
    LoadClientsConfig load_config_;
};


} // namespace will
