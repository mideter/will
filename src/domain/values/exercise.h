#pragma once

#include "values/approach.h"

#include <cstddef>
#include <string>
#include <vector>


namespace will::domain {


/// Exercise (Упражнение) — named freely by the testator, done in approaches, each
/// with its own weight and repetitions.
class Exercise {
public:
	static constexpr std::size_t MaxNameLength = 128;
	static constexpr std::size_t MaxApproaches = 100;

	/// Throws std::invalid_argument if the name is empty or too long, or there
	/// are no approaches or too many.
	Exercise(std::string name, std::vector<Approach> approaches);

	const std::string& name() const noexcept { return name_; }
	const std::vector<Approach>& approaches() const noexcept { return approaches_; }

	bool operator==(const Exercise&) const = default;

private:
	std::string name_;
	std::vector<Approach> approaches_;
};


} // namespace will::domain
