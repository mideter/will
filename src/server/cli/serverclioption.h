#pragma once

#include "clioption.h"
#include "serverconfig.h"

#include <array>
#include <functional>
#include <span>
#include <string_view>
#include <variant>


namespace will {


template<typename ValueTag>
class ServerCliOption : public CliOption<ValueTag> {
public:
    using Value = std::variant<std::monostate, int, std::size_t>;
    using Applier = std::function<void(ServerConfig&, const Value&)>;

    ServerCliOption(Applier applier, std::string_view flag, CliUsagePrinter print_usage,
                    std::span<const std::string_view> aliases = {});

    void apply(ServerConfig& config, const Value& value) const { applier_(config, value); }

private:
    Applier applier_;
};


using ServerOption = std::variant<ServerCliOption<IntValue>, ServerCliOption<SizeValue>>;


struct ServerCliOptionTable {
    static const std::array<ServerOption, 5> ServerOptions;
    static const CliOption<NoneValue> HelpOption;
};


} // namespace will
