#include "message_id.h"

#include <stdexcept>


namespace will::domain {


MessageId::MessageId(const std::uint64_t value) : Id(value)
{
    if (value == 0)
        throw std::invalid_argument("MessageId must be non-zero");
}


} // namespace will::domain
