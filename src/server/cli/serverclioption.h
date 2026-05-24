#pragma once

#include "clioption.h"
#include "serverconfig.h"

#include <array>
#include <functional>
#include <span>
#include <string_view>


namespace will {


class ServerCliOption : public CliOption {
public:
    using Applier = std::function<void(ServerConfig&, const CliOptionMatch::Value&)>;

    ServerCliOption(Applier applier, std::string_view flag, CliValueType value_type,
                    UsagePrinter print_usage, std::span<const std::string_view> aliases = {});

    void apply(ServerConfig& config, const CliOptionMatch::Value& value) const;

    static const std::array<ServerCliOption, 5> ServerOptions;
    static const CliOption HelpOption;

private:
    Applier applier_;
};


} // namespace will
