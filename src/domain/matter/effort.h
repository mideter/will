#pragma once

#include "identity/word.h"
#include "values/timestamp.h"
#include "values/weight.h"

#include <cstdint>


namespace will::domain::matter {


/// Effort (Усилие) — an approach of a training done: which one (the exercise
/// and the approach by their place in the training), with what weight and how
/// many repetitions, begun and finished when. Temporality keeps it, for it is
/// what happened in time; an approach is done at most once. The rest before it
/// is the time since the effort before it finished.
class Effort {
public:
	/// Throws std::invalid_argument unless 1 <= repetitions and it does not
	/// finish before it begins.
	Effort(id::Word training, std::uint32_t exercise, std::uint32_t approach, Weight weight,
		   std::uint32_t repetitions, Timestamp begun, Timestamp finished);

	/// The training (the word of its behest) the approach is of.
	id::Word training() const noexcept { return training_; }
	/// The exercise and the approach, by their place, counted from zero.
	std::uint32_t exercise() const noexcept { return exercise_; }
	std::uint32_t approach() const noexcept { return approach_; }
	Weight weight() const noexcept { return weight_; }
	std::uint32_t repetitions() const noexcept { return repetitions_; }
	Timestamp begun() const noexcept { return begun_; }
	Timestamp finished() const noexcept { return finished_; }

private:
	id::Word training_;
	std::uint32_t exercise_;
	std::uint32_t approach_;
	Weight weight_;
	std::uint32_t repetitions_;
	Timestamp begun_;
	Timestamp finished_;
};


} // namespace will::domain::matter
