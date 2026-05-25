#pragma once

#include "loadclientsconfig.h"


namespace will {
namespace cli {


class LoadClientsConfigParser {
public:
    LoadClientsConfigParser(int argc, char* argv[]);

    const LoadClientsConfig& load_config() const noexcept;

private:
    LoadClientsConfig load_config_;
};


} // namespace cli
} // namespace will
