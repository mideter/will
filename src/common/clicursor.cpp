#include "clicursor.h"

#include <charconv>
#include <cstdlib>
#include <iostream>


namespace will {


CliCursor::CliCursor(int argc, char* argv[])
    : argc_(argc)
    , argv_(argv)
{}


void CliCursor::begin_options() noexcept
{
    index_ = 1;
}


bool CliCursor::has_option() const noexcept
{
    return index_ < argc_;
}


void CliCursor::next_option() noexcept
{
    ++index_;
}


std::string_view CliCursor::current_option() const
{
    return std::string_view{argv_[index_]};
}


std::string_view CliCursor::need_value(std::string_view flag)
{
    if (index_ + 1 >= argc_) {
        std::cerr << flag << " requires a value\n";
        std::exit(2);
    }

    return std::string_view{argv_[++index_]};
}


std::optional<std::size_t> CliCursor::parse_size(std::string_view text)
{
    std::size_t value = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);

    if (ec != std::errc{} || ptr != text.data() + text.size())
        return std::nullopt;

    return value;
}


std::optional<int> CliCursor::parse_int(std::string_view text)
{
    int value = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);

    if (ec != std::errc{} || ptr != text.data() + text.size())
        return std::nullopt;

    return value;
}


void CliCursor::cli_fail_flag(std::string_view flag) const
{
    std::cerr << "Invalid " << flag << '\n';
    std::exit(2);
}


void CliCursor::cli_fail_option(std::string_view flag, const std::exception& error) const
{
    std::cerr << "Invalid " << flag << ": " << error.what() << '\n';
    std::exit(2);
}


int CliCursor::require_int(std::string_view flag)
{
    const auto value = parse_int(need_value(flag));

    if (!value)
        cli_fail_flag(flag);

    return *value;
}


std::size_t CliCursor::require_size(std::string_view flag)
{
    const auto value = parse_size(need_value(flag));

    if (!value)
        cli_fail_flag(flag);

    return *value;
}


} // namespace will
