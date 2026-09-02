#pragma once

#include <compare>
#include <cstdint>
#include <functional>


namespace will::domain {


/// Common base for strongly-typed persistent identifiers.
class Id {
public:
    constexpr std::uint64_t value() const noexcept { return value_; }

    constexpr auto operator<=>(const Id&) const noexcept = default;
    constexpr bool operator==(const Id&) const noexcept = default;

protected:
    explicit constexpr Id(std::uint64_t value) noexcept : value_(value) {}

private:
    std::uint64_t value_;
};


template<typename IdType>
struct IdHash {
    std::size_t operator()(const IdType& id) const noexcept
    {
        return std::hash<std::uint64_t>{}(id.value());
    }
};


} // namespace will::domain
