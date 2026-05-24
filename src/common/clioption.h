#pragma once

#include "clioptioncursor.h"

#include <array>
#include <concepts>
#include <cstddef>
#include <optional>
#include <functional>
#include <iosfwd>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <variant>


namespace will {
namespace cli {


class Value {};


class IntValue : public Value {
public:
    static int read(OptionCursor& cursor, std::string_view flag);

private:
    static std::optional<int> parse(std::string_view text);
};


class SizeValue : public Value {
public:
    static std::size_t read(OptionCursor& cursor, std::string_view flag);

private:
    static std::optional<std::size_t> parse(std::string_view text);
};


class NoneValue : public Value {
public:
    static std::monostate read(OptionCursor& cursor, std::string_view flag);
};


class Error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};


class UnknownOptionError : public Error {
public:
    explicit UnknownOptionError(std::string_view token);
};


class HelpNotAloneError : public Error {
public:
    HelpNotAloneError();
};


class InvalidOptionError : public Error {
public:
    InvalidOptionError(std::string_view flag, std::string_view reason);
};


using UsagePrinter = std::function<void(std::ostream&)>;
using ParsedValue = std::variant<std::monostate, int, std::size_t>;


template<typename OptionVariant>
class OptionMatch {
public:
    using Value = ParsedValue;

    template<std::size_t N>
    OptionMatch(OptionCursor& cursor, const std::array<OptionVariant, N>& options);

    std::string_view primary_flag() const;
    const Value& value() const noexcept { return value_; }
    const OptionVariant& option() const noexcept { return *option_; }

private:
    const OptionVariant* option_ = nullptr;
    Value value_;
};


class OptionBase {
public:
    virtual ~OptionBase() = default;

    bool matches(std::string_view text) const;
    std::string_view primary_flag() const noexcept;
    void print_usage(std::ostream& os) const;

protected:
    OptionBase(std::string_view flag, UsagePrinter print_usage,
                  std::span<const std::string_view> aliases = {});

    std::string_view flag_;
    UsagePrinter print_usage_;
    std::span<const std::string_view> aliases_;
};


template<typename ValueTag>
    requires std::derived_from<ValueTag, Value>
class Option : public OptionBase {
public:
    Option(std::string_view flag, UsagePrinter print_usage,
              std::span<const std::string_view> aliases = {})
        : OptionBase(flag, std::move(print_usage), aliases)
    {}

    ParsedValue read_value(OptionCursor& cursor) const
    {
        return ParsedValue{ValueTag::read(cursor, primary_flag())};
    }
};


template<typename OptionVariant>
template<std::size_t N>
OptionMatch<OptionVariant>::OptionMatch(OptionCursor& cursor,
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

    throw UnknownOptionError(text);
}


template<typename OptionVariant>
std::string_view OptionMatch<OptionVariant>::primary_flag() const
{
    return std::visit([](const auto& option) { return option.primary_flag(); }, *option_);
}


} // namespace cli
} // namespace will
