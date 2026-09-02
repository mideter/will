#pragma once

#include "id.h"

#include <compare>


namespace will::domain {


/// Persistent message identity assigned by storage.
class MessageId : public Id {
public:
    explicit MessageId(std::uint64_t value) : Id(value) {}

    constexpr auto operator<=>(const MessageId&) const noexcept = default;
    constexpr bool operator==(const MessageId&) const noexcept = default;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::MessageId> : will::domain::IdHash<will::domain::MessageId> {};
