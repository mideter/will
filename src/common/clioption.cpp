#include "clioption.h"

#include "clicursor.h"

#include <format>


namespace will {


CliUnknownOptionError::CliUnknownOptionError(const std::string_view token)
    : CliError(std::format("Unknown option: {}", token))
{}


CliHelpNotAloneError::CliHelpNotAloneError()
    : CliError("--help must be the only option")
{}


CliInvalidOptionError::CliInvalidOptionError(const std::string_view flag,
                                             const std::string_view reason)
    : std::runtime_error(std::format("Invalid {}: {}", flag, reason))
{}


CliOption::CliOption(const std::string_view flag, const CliValueType value_type,
                     const UsagePrinter print_usage, const std::span<const std::string_view> aliases)
    : flag_(flag)
    , value_type_(value_type)
    , print_usage_(print_usage)
    , aliases_(aliases)
{}


bool CliOption::matches(std::string_view text) const
{
    if (text == flag_)
        return true;

    for (const std::string_view alias : aliases_) {
        if (text == alias)
            return true;
    }

    return false;
}


std::string_view CliOption::primary_flag() const noexcept
{
    return flag_;
}


void CliOption::print_usage(std::ostream& os) const
{
    print_usage_(os);
}


CliValueType CliOption::value_type() const noexcept
{
    return value_type_;
}


CliOptionMatch::CliOptionMatch(const CliOption& option, const std::string_view token)
    : option_(&option)
    , token_(token)
{}


CliOptionMatch CliOptionMatch::parse(CliCursor& cursor, const std::span<const CliOption> options)
{
    const std::string_view text = cursor.current_option();

    for (const CliOption& option : options) {
        if (!option.matches(text))
            continue;

        CliOptionMatch match(option, text);
        match.read_value(cursor);
        return match;
    }

    throw CliUnknownOptionError(text);
}


void CliOptionMatch::read_value(CliCursor& cursor)
{
    const std::string_view flag = option_->primary_flag();

    switch (option_->value_type()) {
    case CliValueType::Int:
        int_value_ = cursor.require_int(flag);
        break;
    case CliValueType::Size:
        size_value_ = cursor.require_size(flag);
        break;
    case CliValueType::None:
        break;
    }
}


} // namespace will
