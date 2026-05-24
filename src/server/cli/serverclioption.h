#pragma once

#include "clioption.h"
#include "serverconfig.h"

#include <span>


namespace will {


struct ServerCliOption {
    CliOption cli;
    void (*apply)(ServerConfig& config, const CliOptionMatch::Value& value);
};


std::span<const ServerCliOption> ServerCliOptions();
std::span<const CliOption> ServerCliOptionCliOptions();

extern const CliOption HelpCliOption;


} // namespace will
