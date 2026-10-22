#pragma once

#include "identity/word.h"
#include "values/timestamp.h"

#include <cstdint>


namespace will::domain::matter {


/// An approach begun and not yet finished: an effort without its end. Exercises and
/// approaches are counted from zero.
class Underway {
public:
	Underway(id::Word training, std::uint32_t exercise, std::uint32_t approach, Timestamp begun);

	id::Word training() const noexcept { return training_; }
	std::uint32_t exercise() const noexcept { return exercise_; }
	std::uint32_t approach() const noexcept { return approach_; }
	Timestamp begun() const noexcept { return begun_; }

private:
	id::Word training_;
	std::uint32_t exercise_;
	std::uint32_t approach_;
	Timestamp begun_;
};


} // namespace will::domain::matter
