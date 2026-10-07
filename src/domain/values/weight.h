#pragma once

#include <cstdint>


namespace will::domain {


/// Weight (Вес) — the load of an approach, in grams: whole grams keep 62.5 kg
/// exact. Zero is one's own weight: the body itself is the load.
class Weight {
public:
	/// At most a ton.
	static constexpr std::uint32_t MaxGrams = 1'000'000;

	/// Throws std::invalid_argument if heavier than MaxGrams.
	explicit Weight(std::uint32_t grams);

	std::uint32_t grams() const noexcept { return grams_; }

	/// The body itself is the load.
	bool own() const noexcept { return grams_ == 0; }

	bool operator==(const Weight&) const = default;

private:
	std::uint32_t grams_;
};


} // namespace will::domain
