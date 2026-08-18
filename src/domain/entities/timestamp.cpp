module;

#include <chrono>

module will.domain.timestamp;


using std::chrono::duration_cast;
using std::chrono::nanoseconds;
using std::chrono::system_clock;


namespace will::domain {


Timestamp::Timestamp() noexcept = default;


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
