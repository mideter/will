module;

#include <compare>
#include <cstdint>

export module will.domain.participant_id;

export namespace will::domain {


/// Domain identifier for a connected participant (not IP, not socket handle).
struct ParticipantId {
    std::uint64_t value = 0;

    ParticipantId() noexcept;
    constexpr explicit ParticipantId(std::uint64_t v) noexcept : value(v) {}

    constexpr auto operator<=>(const ParticipantId&) const noexcept = default;
    constexpr bool operator==(const ParticipantId&) const noexcept = default;
};


ParticipantId::ParticipantId() noexcept = default;


} // namespace will::domain
