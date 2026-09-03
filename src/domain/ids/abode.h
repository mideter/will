#pragma once

#include "id.h"

#include <compare>


namespace will::domain::id {


/// Persistent abode identity. The single global abode is id 1 (see global()).
class Abode : public Id {
public:
    explicit Abode(std::uint64_t value) : Id(value) {}

    static Abode global() { return Abode{1}; }

    constexpr auto operator<=>(const Abode&) const noexcept = default;
    constexpr bool operator==(const Abode&) const noexcept = default;
};


} // namespace will::domain::id


template <>
struct std::hash<will::domain::id::Abode> : will::domain::id::IdHash<will::domain::id::Abode> {};
