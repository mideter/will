#pragma once

#include "identity/word.h"
#include "values/timestamp.h"
#include "values/strain.h"
#include "values/weight.h"

#include <cstdint>


namespace will::domain::matter {


class Effort {
public:
	Effort(id::Word training, std::uint32_t exercise, std::uint32_t approach, Weight weight,
		   std::uint32_t repetitions, Timestamp begun, Timestamp finished, Strain strain = Strain{});

	id::Word training() const noexcept { return training_; }
	std::uint32_t exercise() const noexcept { return exercise_; }
	std::uint32_t approach() const noexcept { return approach_; }
	Weight weight() const noexcept { return weight_; }
	std::uint32_t repetitions() const noexcept { return repetitions_; }
	Timestamp begun() const noexcept { return begun_; }
	Timestamp finished() const noexcept { return finished_; }
	Strain strain() const noexcept { return strain_; }

private:
	id::Word training_;
	std::uint32_t exercise_;
	std::uint32_t approach_;
	Weight weight_;
	std::uint32_t repetitions_;
	Timestamp begun_;
	Timestamp finished_;
	Strain strain_;
};


} // namespace will::domain::matter
