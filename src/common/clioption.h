#pragma once

#include "cliparsercontext.h"

#include <array>
#include <cstddef>
#include <iosfwd>
#include <string_view>


namespace will {


template<typename Config>
class CliOptionBase {
public:
    virtual ~CliOptionBase() = default;

    [[nodiscard]] virtual bool matches(std::string_view text) const = 0;
    [[nodiscard]] virtual std::string_view primary_flag() const = 0;

    virtual void print_usage(std::ostream& os) const = 0;
    virtual void apply(CliParserContext<Config>& context) const = 0;
};


template<typename Config, typename Derived>
class CliOption : public CliOptionBase<Config> {
public:
    [[nodiscard]] static const Derived& instance()
    {
        static const Derived option;
        return option;
    }

protected:
    CliOption() = default;
    ~CliOption() = default;
};


template<typename Config, std::size_t N>
[[nodiscard]] const CliOptionBase<Config>* find_cli_option(const std::array<const CliOptionBase<Config>*, N>& options,
                                                            std::string_view text)
{
    for (const CliOptionBase<Config>* option : options) {
        if (option->matches(text))
            return option;
    }

    return nullptr;
}


template<typename Config, std::size_t N>
void print_cli_usage(std::ostream& os, const std::array<const CliOptionBase<Config>*, N>& options)
{
    for (const CliOptionBase<Config>* option : options)
        option->print_usage(os);
}


} // namespace will
