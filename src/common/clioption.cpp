#include "clioption.h"

#include <format>
#include <ostream>


namespace will {
namespace cli {


int IntValue::read(OptionCursor& cursor, const std::string_view flag)
{
    return cursor.require_int(flag);
}


std::size_t SizeValue::read(OptionCursor& cursor, const std::string_view flag)
{
    return cursor.require_size(flag);
}


std::monostate NoneValue::read(OptionCursor& cursor, const std::string_view flag)
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
