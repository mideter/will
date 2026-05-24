#pragma once

#include "clicursor.h"

#include <array>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <span>
#include <stdexcept>
#include <string_view>
#include <variant>


namespace will {


enum class CliValueType {
    None,
    Int,
    Size,
};


class CliError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};


class CliUnknownOptionError : public CliError {
public:
    explicit CliUnknownOptionError(std::string_view token);
};


class CliHelpNotAloneError : public CliError {
public:
    CliHelpNotAloneError();
};


class CliInvalidOptionError : public CliError {
public:
    CliInvalidOptionError(std::string_view flag, std::string_view reason);
};


class CliOption {
public:
    using UsagePrinter = std::function<void(std::ostream&)>;

    CliOption(std::string_view flag, CliValueType value_type, UsagePrinter print_usage,
              std::span<const std::string_view> aliases = {});

    bool matches(std::string_view text) const;
    std::string_view primary_flag() const noexcept;
    void print_usage(std::ostream& os) const;
    CliValueType value_type() const noexcept;

private:
    std::string_view flag_;
    CliValueType value_type_;
    UsagePrinter print_usage_;
    std::span<const std::string_view> aliases_;
};


class CliOptionMatch {
public:
    using Value = std::variant<std::monostate, int, std::size_t>;

    template<std::derived_from<CliOption> Option, std::size_t N>
    CliOptionMatch(CliCursor& cursor, const std::array<Option, N>& options);

    std::string_view primary_flag() const { return option_->primary_flag(); }
    const CliOption& option() const noexcept { return *option_; }
    const Value& value() const noexcept { return value_; }

private:
    void read_value(CliCursor& cursor);

    const CliOption* option_;
    Value value_;
};


template<std::derived_from<CliOption> Option, std::size_t N>
CliOptionMatch::CliOptionMatch(CliCursor& cursor, const std::array<Option, N>& options)
{
    const std::string_view text = cursor.current_option();

    for (const Option& option : options) {
        if (!option.matches(text))
            continue;

        option_ = &static_cast<const CliOption&>(option);
        read_value(cursor);
        return;
    }

    throw CliUnknownOptionError(text);
}


} // namespace will
