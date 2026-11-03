#pragma once

#include <cstdint>


namespace will::domain {


class Strain {
public:
	static constexpr std::uint32_t MaxMarks = 2;

	explicit Strain(std::uint32_t marks = 0);

	std::uint32_t marks() const noexcept { return marks_; }

	bool operator==(const Strain&) const = default;

private:
	std::uint32_t marks_;
};


} // namespace will::domain
