module;

#include <compare>
#include <cstdint>

export module will.domain.user_id;

export namespace will::domain {


struct UserId {
    std::uint64_t value = 0;

    UserId() noexcept;
    constexpr explicit UserId(std::uint64_t v) noexcept : value(v) {}

    constexpr auto operator<=>(const UserId&) const noexcept = default;
    constexpr bool operator==(const UserId&) const noexcept = default;
};


UserId::UserId() noexcept = default;


} // namespace will::domain
