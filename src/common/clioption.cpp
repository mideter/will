#include "clioption.h"

#include <charconv>
#include <format>
#include <ostream>


namespace will {
namespace cli {


std::optional<int> IntValue::parse(std::string_view text)
{
    int value = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);

    if (ec != std::errc{} || ptr != text.data() + text.size())
        return std::nullopt;

    return value;
}


std::optional<std::size_t> SizeValue::parse(std::string_view text)
{
    std::size_t value = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);

    if (ec != std::errc{} || ptr != text.data() + text.size())
        return std::nullopt;

    return value;
}


int IntValue::read(OptionCursorCore& cursor, const std::string_view flag)
{
    const auto value = parse(cursor.need_value(flag));

    if (!value)
        throw InvalidOptionError(flag, "invalid value");

    return *value;
}


std::size_t SizeValue::read(OptionCursorCore& cursor, const std::string_view flag)
{
    const auto value = parse(cursor.need_value(flag));

    if (!value)
        throw InvalidOptionError(flag, "invalid value");

    return *value;
}


std::monostate NoneValue::read(OptionCursorCore& cursor, const std::string_view flag)
{
    (void)cursor;
    (void)flag;
    return std::monostate{};
}


UnknownOptionError::UnknownOptionError(const std::string_view token)
    : Error(std::format("Unknown option: {}", token))
{}


HelpNotAloneError::HelpNotAloneError()
    : Error("--help must be the only option")
{}


InvalidOptionError::InvalidOptionError(const std::string_view flag,
                                             const std::string_view reason)
    : Error(std::format("Invalid {}: {}", flag, reason))
{}


bool OptionBase::matches(std::string_view text) const
{
    if (text == flag_)
        return true;

    for (const std::string_view alias : aliases_) {
        if (text == alias)
            return true;
    }

    return false;
}


OptionBase::OptionBase(const std::string_view flag, UsagePrinter print_usage,
                             const std::span<const std::string_view> aliases)
    : flag_(flag)
    , print_usage_(std::move(print_usage))
    , aliases_(aliases)
{}


std::string_view OptionBase::primary_flag() const noexcept
{
    return flag_;
}


void OptionBase::print_usage(std::ostream& os) const
{
    print_usage_(os);
}


} // namespace cli
} // namespace will
