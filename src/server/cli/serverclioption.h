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
    requires(std::derived_from<ValueTag, Value> && !std::is_same_v<ValueTag, NoneValue>)
class ServerCliOption : public CliOption<ValueTag> {
public:
    using Applier = std::function<void(ServerConfig&, const CliParsedValue&)>;

    ServerCliOption(Applier applier, 
                    std::string_view flag, 
                    CliUsagePrinter print_usage,
                    std::span<const std::string_view> aliases = {});

    void apply(ServerConfig& config, const CliParsedValue& value) const;

private:
    Applier applier_;
};


using ServerOption = std::variant<ServerCliOption<IntValue>, ServerCliOption<SizeValue>>;


struct ServerCliOptionTable {
    static const std::array<ServerOption, 5> ServerOptions;
    static const CliOption<NoneValue> HelpOption;
};


} // namespace will
