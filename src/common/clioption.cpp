#include "clioption.h"

#include <format>
#include <ostream>


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


bool CliOptionBase::matches(std::string_view text) const
{
    if (text == flag_)
        return true;

    for (const std::string_view alias : aliases_) {
        if (text == alias)
            return true;
    }

    return false;
}


CliOptionBase::CliOptionBase(const std::string_view flag, CliUsagePrinter print_usage,
                             const std::span<const std::string_view> aliases)
    : flag_(flag)
    , print_usage_(std::move(print_usage))
    , aliases_(aliases)
{}


std::string_view CliOptionBase::primary_flag() const noexcept
{
    return flag_;
}


void CliOptionBase::print_usage(std::ostream& os) const
{
    print_usage_(os);
}


} // namespace will
