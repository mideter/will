#include "support/timestamp.h"

#include <chrono>


using std::chrono::system_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;


namespace will::domain {


std::optional<Timestamp> Timestamp::parse(const std::int64_t raw) noexcept
{
    if (raw < 0)
        return std::nullopt;

    return Timestamp{raw};
}


Timestamp Timestamp::now()
{
    auto ns = system_clock::now().time_since_epoch();
    std::int64_t ms = duration_cast<milliseconds>(ns).count();

    return Timestamp{ms};
}


} // namespace will::domain
