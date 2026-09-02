#pragma once

#include "id.h"

#include <compare>


namespace will::domain {


/// Persistent abode identity. Zero denotes the single global abode (see global()).
class AbodeId : public Id {
public:
    explicit constexpr AbodeId(std::uint64_t value) noexcept : Id(value) {}

    static constexpr AbodeId global() noexcept { return AbodeId{0}; }

    constexpr bool is_global() const noexcept { return value() == 0; }

    constexpr auto operator<=>(const AbodeId&) const noexcept = default;
    constexpr bool operator==(const AbodeId&) const noexcept = default;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::AbodeId> : will::domain::IdHash<will::domain::AbodeId> {};
