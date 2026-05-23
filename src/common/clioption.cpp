#include "clioption.h"

#include "clicursor.h"


namespace will {


CliOption::CliOption(std::string_view flag, CliValueType value_type)
    : flag_(flag)
    , value_type_(value_type)
{}


bool CliOption::matches(std::string_view text) const
{
    return text == flag_;
}


std::string_view CliOption::primary_flag() const
{
    return flag_;
}


std::string_view CliOption::flag() const noexcept
{
    return flag_;
}


void CliOptionMatch::read_value(CliCursor& cursor)
{
    if (!option_)
        return;

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
