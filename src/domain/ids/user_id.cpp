#include "user_id.h"

#include <stdexcept>


namespace will::domain {


UserId::UserId(const std::uint64_t value) : Id(value)
{
    if (value == 0)
        throw std::invalid_argument("UserId must be non-zero");
}


} // namespace will::domain
