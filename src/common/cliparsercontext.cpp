#include "cliparsercontext.h"

#include <charconv>
#include <cstdlib>
#include <iostream>


namespace will {


CliParserContextBase::CliParserContextBase(int argc, char* argv[])
    : argc_(argc)
    , argv_(argv)
{}


std::string_view CliParserContextBase::current() const
{
    return std::string_view{argv_[index_]};
}


std::string_view CliParserContextBase::need_value(std::string_view flag)
{
    if (index_ + 1 >= argc_) {
        std::cerr << flag << " requires a value\n";
        std::exit(2);
    }

    return std::string_view{argv_[++index_]};
}


std::optional<std::size_t> CliParserContextBase::parse_size(std::string_view text)
{
    std::size_t value = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);

    if (ec != std::errc{} || ptr != text.data() + text.size())
        return std::nullopt;

    return value;
}


std::optional<int> CliParserContextBase::parse_int(std::string_view text)
{
    int value = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);

    if (ec != std::errc{} || ptr != text.data() + text.size())
        return std::nullopt;

    return value;
}


void CliParserContextBase::cli_fail_flag(std::string_view flag) const
{
    std::cerr << "Invalid " << flag << '\n';
    std::exit(2);
}


void CliParserContextBase::cli_fail_option(std::string_view flag, const std::exception& error) const
{
    std::cerr << "Invalid " << flag << ": " << error.what() << '\n';
    std::exit(2);
}


int CliParserContextBase::require_int(std::string_view flag)
{
    const auto value = parse_int(need_value(flag));

    if (!value)
        cli_fail_flag(flag);

    return *value;
}


std::size_t CliParserContextBase::require_size(std::string_view flag)
{
    const auto value = parse_size(need_value(flag));

    if (!value)
        cli_fail_flag(flag);

    return *value;
}


} // namespace will
