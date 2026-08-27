#include "message_id.h"

#include <stdexcept>


namespace will::domain {


MessageId::MessageId(const std::uint64_t value)
{
    if (value == 0)
        throw std::invalid_argument("MessageId must be non-zero");

    value_ = value;
}


} // namespace will::domain
