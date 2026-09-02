#pragma once

#include "id.h"

#include <compare>


namespace will::domain {


/// Persistent abode identity. The single global abode is id 1 (see global()).
class AbodeId : public Id {
public:
    explicit AbodeId(std::uint64_t value) : Id(value) {}

    static AbodeId global() { return AbodeId{1}; }

    constexpr auto operator<=>(const AbodeId&) const noexcept = default;
    constexpr bool operator==(const AbodeId&) const noexcept = default;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::AbodeId> : will::domain::IdHash<will::domain::AbodeId> {};
