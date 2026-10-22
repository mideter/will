#pragma once

#include "identity/word.h"
#include "values/exercise.h"

#include <cstddef>
#include <vector>


namespace will::domain::matter {


/// What a behest wills, or what the deed fulfilling it has done.
class Training {
public:
	static constexpr std::size_t MaxExercises = 50;

	Training(id::Word word, std::vector<Exercise> exercises);

	id::Word id() const noexcept { return word_; }
	const std::vector<Exercise>& exercises() const noexcept { return exercises_; }

private:
	id::Word word_;
	std::vector<Exercise> exercises_;
};


} // namespace will::domain::matter
