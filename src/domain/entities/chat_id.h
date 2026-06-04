#pragma once

#include <cstdint>
#include <compare>


namespace will::domain {


struct ChatId {
    std::uint64_t value = 0;

    constexpr ChatId() noexcept = default;
    constexpr explicit ChatId(std::uint64_t v) noexcept : value(v) {}

    static constexpr ChatId global() noexcept { return ChatId{0}; }

    constexpr auto operator<=>(const ChatId&) const noexcept = default;
    constexpr bool operator==(const ChatId&) const noexcept = default;
};


} // namespace will::domain
