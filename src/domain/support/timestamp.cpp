#include "support/timestamp.h"

#include <chrono>


using std::chrono::system_clock;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;


namespace will::domain {


std::optional<Timestamp> Timestamp::parse(const std::int64_t raw) noexcept
{
    if (raw < 0)
        return std::nullopt;

    return Timestamp{raw};
}


Timestamp Timestamp::now()
{
    auto tse = system_clock::now().time_since_epoch();
    auto ns = duration_cast<nanoseconds>(tse).count();
    return Timestamp{ns};
}


} // namespace will::domain
