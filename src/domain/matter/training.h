#pragma once

#include "identity/word.h"
#include "values/exercise.h"

#include <cstddef>
#include <vector>


namespace will::domain::matter {


/// Training (Тренировка) — the exercises of a word in a tie: what a behest wills,
/// or what a deed fulfilling it has done. Eternity keeps it beside the word, for
/// what a word says does not change.
class Training {
public:
	static constexpr std::size_t MaxExercises = 50;

	/// Throws std::invalid_argument if there are no exercises or too many.
	Training(id::Word word, std::vector<Exercise> exercises);

	id::Word id() const noexcept { return word_; }
	const std::vector<Exercise>& exercises() const noexcept { return exercises_; }

private:
	id::Word word_;
	std::vector<Exercise> exercises_;
};


} // namespace will::domain::matter
