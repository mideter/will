#include "clioptioncursor.h"

#include "clioption.h"

#include <charconv>


namespace will {
namespace cli {


OptionCursor::OptionCursor(int argc, char* argv[])
    : argc_(argc)
    , argv_(argv)
{}


bool OptionCursor::has_option() const noexcept
{
    return index_ < argc_;
}


OptionCursor OptionCursor::operator++(int) noexcept
{
    OptionCursor before{*this};

    ++index_;

    if (current_option_has_value_)
        ++index_;

    current_option_has_value_ = false;
    return before;
}


std::string_view OptionCursor::current_option() const
{
    return std::string_view{argv_[index_]};
}


std::string_view OptionCursor::need_value(std::string_view flag)
{
    if (index_ + 1 >= argc_)
        throw InvalidOptionError(flag, "requires a value");

    current_option_has_value_ = true;
    return std::string_view{argv_[index_ + 1]};
}


std::optional<std::size_t> OptionCursor::parse_size(std::string_view text)
{
    std::size_t value = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);

    if (ec != std::errc{} || ptr != text.data() + text.size())
        return std::nullopt;

    return value;
}


std::optional<int> OptionCursor::parse_int(std::string_view text)
{
    int value = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);

    if (ec != std::errc{} || ptr != text.data() + text.size())
        return std::nullopt;

    return value;
}


void OptionCursor::fail_flag(std::string_view flag) const
{
    throw InvalidOptionError(flag, "invalid value");
}


int OptionCursor::require_int(std::string_view flag)
{
    const auto value = parse_int(need_value(flag));

    if (!value)
        fail_flag(flag);

    return *value;
}


std::size_t OptionCursor::require_size(std::string_view flag)
{
    const auto value = parse_size(need_value(flag));

    if (!value)
        fail_flag(flag);

    return *value;
}


} // namespace cli
} // namespace will
