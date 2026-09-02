#pragma once

#include <compare>
#include <cstdint>
#include <functional>


namespace will::domain {


/// Persistent chat identity. Zero denotes the single global chat (see global()).
class ChatId {
public:
    explicit constexpr ChatId(std::uint64_t value) noexcept : value_(value) {}

    static constexpr ChatId global() noexcept { return ChatId{0}; }

    constexpr std::uint64_t value() const noexcept { return value_; }
    constexpr bool is_global() const noexcept { return value_ == 0; }

    constexpr auto operator<=>(const ChatId&) const noexcept = default;
    constexpr bool operator==(const ChatId&) const noexcept = default;

private:
    std::uint64_t value_;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::ChatId> {
    std::size_t operator()(const will::domain::ChatId id) const noexcept
    {
        return std::hash<std::uint64_t>{}(id.value());
    }
};
