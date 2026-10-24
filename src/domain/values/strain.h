#pragma once

#include <cstdint>


namespace will::domain {


/// How an approach went, in marks as one writes them by it: none — it went as it should; one
/// — it was hard; two — it was at the limit.
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
