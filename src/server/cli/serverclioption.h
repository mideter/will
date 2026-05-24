#pragma once

#include "clioption.h"
#include "serverconfig.h"

#include <array>
#include <functional>
#include <span>
#include <string_view>
#include <variant>


namespace will {
namespace cli {


template<typename ValueTag>
    requires(std::derived_from<ValueTag, Value> && !std::is_same_v<ValueTag, NoneValue>)
class ConfigOption : public Option<ValueTag> {
public:
    using Applier = std::function<void(ServerConfig&, const ParsedValue&)>;

    ConfigOption(Applier applier, 
                    std::string_view flag, 
                    UsagePrinter print_usage,
                    std::span<const std::string_view> aliases = {});

    void apply(ServerConfig& config, const ParsedValue& value) const;

private:
    Applier applier_;
};


using ServerOption = std::variant<ConfigOption<IntValue>, ConfigOption<SizeValue>>;


struct ServerOptionTable {
    static const std::array<ServerOption, 5> ServerOptions;
    static const Option<NoneValue> HelpOption;
};


} // namespace cli
} // namespace will
