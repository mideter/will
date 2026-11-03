#pragma once

#include <compare>
#include <cstdint>


namespace will {


struct SessionId {
	std::uint64_t value = 0;

	constexpr SessionId() noexcept = default;
	constexpr explicit SessionId(const std::uint64_t v) noexcept : value(v) {}

	constexpr auto operator<=>(const SessionId&) const noexcept = default;
	constexpr bool operator==(const SessionId&) const noexcept = default;
};


} // namespace will
