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
#include <utility>
#include <variant>


namespace will {


class Value {};


class IntValue : public Value {
public:
    static int read(CliCursor& cursor, std::string_view flag);
};


class SizeValue : public Value {
public:
    static std::size_t read(CliCursor& cursor, std::string_view flag);
};


class NoneValue : public Value {
public:
    static std::monostate read(CliCursor& cursor, std::string_view flag);
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


using CliUsagePrinter = std::function<void(std::ostream&)>;
using CliParsedValue = std::variant<std::monostate, int, std::size_t>;


template<typename OptionVariant>
class CliOptionMatch {
public:
    using Value = CliParsedValue;

    template<std::size_t N>
    CliOptionMatch(CliCursor& cursor, const std::array<OptionVariant, N>& options);

    std::string_view primary_flag() const;
    const Value& value() const noexcept { return value_; }
    const OptionVariant& option() const noexcept { return *option_; }

private:
    const OptionVariant* option_ = nullptr;
    Value value_;
};


class CliOptionBase {
public:
    virtual ~CliOptionBase() = default;

    bool matches(std::string_view text) const;
    std::string_view primary_flag() const noexcept;
    void print_usage(std::ostream& os) const;

protected:
    CliOptionBase(std::string_view flag, CliUsagePrinter print_usage,
                  std::span<const std::string_view> aliases = {});

    std::string_view flag_;
    CliUsagePrinter print_usage_;
    std::span<const std::string_view> aliases_;
};


template<typename ValueTag>
    requires std::derived_from<ValueTag, Value>
class CliOption : public CliOptionBase {
public:
    CliOption(std::string_view flag, CliUsagePrinter print_usage,
              std::span<const std::string_view> aliases = {})
        : CliOptionBase(flag, std::move(print_usage), aliases)
    {}

    CliParsedValue read_value(CliCursor& cursor) const
    {
        return CliParsedValue{ValueTag::read(cursor, primary_flag())};
    }
};


template<typename OptionVariant>
template<std::size_t N>
CliOptionMatch<OptionVariant>::CliOptionMatch(CliCursor& cursor,
                                              const std::array<OptionVariant, N>& options)
{
    const std::string_view text = cursor.current_option();

    for (const OptionVariant& option : options) {
        const bool matched =
            std::visit([&](const auto& candidate) { return candidate.matches(text); }, option);

        if (!matched)
            continue;

        option_ = &option;
        value_ = std::visit([&](const auto& candidate) { return candidate.read_value(cursor); },
                            option);
        return;
    }

    throw CliUnknownOptionError(text);
}


template<typename OptionVariant>
std::string_view CliOptionMatch<OptionVariant>::primary_flag() const
{
    return std::visit([](const auto& option) { return option.primary_flag(); }, *option_);
}


} // namespace will
