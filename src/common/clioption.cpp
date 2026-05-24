#include "clioption.h"

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
    : CliError(std::format("Invalid {}: {}", flag, reason))
{}


CliOption::CliOption(const std::string_view flag, const CliValueType value_type,
                     UsagePrinter print_usage, const std::span<const std::string_view> aliases)
    : flag_(flag)
    , value_type_(value_type)
    , print_usage_(std::move(print_usage))
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


void CliOptionMatch::read_value(CliCursor& cursor)
{
    const std::string_view flag = option_->primary_flag();

    switch (option_->value_type()) {
    case CliValueType::Int:
        value_.emplace<int>(cursor.require_int(flag));
        break;
    case CliValueType::Size:
        value_.emplace<std::size_t>(cursor.require_size(flag));
        break;
    case CliValueType::None:
        value_ = std::monostate{};
        break;
    }
}


} // namespace will
