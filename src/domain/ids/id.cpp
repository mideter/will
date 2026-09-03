#include "id.h"

#include <stdexcept>


namespace will::domain::id {


Id::Id(const std::uint64_t value) : value_(value)
{
    if (value == 0)
        throw std::invalid_argument("Id must be non-zero");
}


} // namespace will::domain::id
