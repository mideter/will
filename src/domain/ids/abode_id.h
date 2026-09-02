#pragma once

#include <compare>
#include <cstdint>
#include <functional>


namespace will::domain {


/// Persistent abode identity. Zero denotes the single global abode (see global()).
class AbodeId {
public:
    explicit constexpr AbodeId(std::uint64_t value) noexcept : value_(value) {}

    static constexpr AbodeId global() noexcept { return AbodeId{0}; }

    constexpr std::uint64_t value() const noexcept { return value_; }
    constexpr bool is_global() const noexcept { return value_ == 0; }

    constexpr auto operator<=>(const AbodeId&) const noexcept = default;
    constexpr bool operator==(const AbodeId&) const noexcept = default;

private:
    std::uint64_t value_;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::AbodeId> {
    std::size_t operator()(const will::domain::AbodeId id) const noexcept
    {
        return std::hash<std::uint64_t>{}(id.value());
    }
};
