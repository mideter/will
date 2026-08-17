#pragma once

#include <compare>
#include <cstdint>
#include <optional>


namespace will::domain {


/// Unix epoch instant; stored as milliseconds since 1970-01-01 UTC. Must be non-negative.
class Timestamp {
public:
    static std::optional<Timestamp> parse(std::int64_t raw) noexcept;
    static Timestamp now();

    constexpr Timestamp() noexcept = default;

    constexpr std::int64_t value() const noexcept { return value_; }

    constexpr auto operator<=>(const Timestamp&) const noexcept = default;
    constexpr bool operator==(const Timestamp&) const noexcept = default;

private:
    explicit constexpr Timestamp(std::int64_t value) noexcept : value_(value) {}

    std::int64_t value_ = 0;
};


} // namespace will::domain
