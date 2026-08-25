#pragma once

#include <cstdint>
#include <compare>


namespace will::domain {


/// Domain identifier for a connected participant (not IP, not socket handle).
struct ParticipantId {
    std::uint64_t value = 0;

    constexpr ParticipantId() noexcept = default;
    constexpr explicit ParticipantId(std::uint64_t v) noexcept : value(v) {}

    constexpr auto operator<=>(const ParticipantId&) const noexcept = default;
    constexpr bool operator==(const ParticipantId&) const noexcept = default;
};


} // namespace will::domain
