#include "clioptioncursor.h"

#include "clioption.h"


namespace will {
namespace cli {


OptionCursorCore::OptionCursorCore(int argc, char* argv[])
    : argc_(argc)
    , argv_(argv)
{}


bool OptionCursorCore::has_option() const noexcept
{
    return index_ < argc_;
}


OptionCursorCore OptionCursorCore::operator++(int) noexcept
{
    OptionCursorCore before{*this};

    ++index_;

    if (current_option_has_value_)
        ++index_;

    current_option_has_value_ = false;
    return before;
}


std::string_view OptionCursorCore::current_option() const
{
    return std::string_view{argv_[index_]};
}


std::string_view OptionCursorCore::need_value(std::string_view flag)
{
    if (index_ + 1 >= argc_)
        throw InvalidOptionError(flag, "requires a value");

    current_option_has_value_ = true;
    return std::string_view{argv_[index_ + 1]};
}


} // namespace cli
} // namespace will
