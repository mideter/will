#pragma once

#include "clioption.h"
#include "serverconfig.h"

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

    static constexpr std::size_t ServerOptionCount = 5;
    static const ServerCliOption ServerOptions[ServerOptionCount];

private:
    Applier applier_;
};


extern const CliOption HelpCliOption;


} // namespace will
