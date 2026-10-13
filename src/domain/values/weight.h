#pragma once

#include <cstdint>


namespace will::domain {


/// Weight (Вес) — in grams, that 62.5 kg stays exact; zero is one's own weight.
class Weight {
public:
	static constexpr std::uint32_t MaxGrams = 1'000'000;

	explicit Weight(std::uint32_t grams);

	std::uint32_t grams() const noexcept { return grams_; }

	bool own() const noexcept { return grams_ == 0; }

	bool operator==(const Weight&) const = default;

private:
	std::uint32_t grams_;
};


} // namespace will::domain
