#pragma once

#include <compare>
#include <cstdint>
#include <functional>


namespace will::domain {


/// Persistent message identity assigned by storage. Must be non-zero.
class MessageId {
public:
    /// Throws std::invalid_argument if value is 0.
    explicit MessageId(std::uint64_t value);

    constexpr std::uint64_t value() const noexcept { return value_; }

    constexpr auto operator<=>(const MessageId&) const noexcept = default;
    constexpr bool operator==(const MessageId&) const noexcept = default;

private:
    std::uint64_t value_;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::MessageId> {
    std::size_t operator()(const will::domain::MessageId id) const noexcept
    {
        return std::hash<std::uint64_t>{}(id.value());
    }
};
