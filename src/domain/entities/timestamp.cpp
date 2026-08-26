#include "timestamp.h"

#include <chrono>
#include <stdexcept>


using std::chrono::system_clock;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;


namespace will::domain {


Timestamp::Timestamp(const std::int64_t value)
{
    if (value < 0)
        throw std::invalid_argument("Timestamp must be non-negative");

    value_ = value;
}


Timestamp::Timestamp() noexcept
{
    const auto tse = system_clock::now().time_since_epoch();
    value_ = duration_cast<nanoseconds>(tse).count();
}


} // namespace will::domain
