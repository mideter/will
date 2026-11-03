#pragma once

#include "identity/word.h"
#include "values/timestamp.h"

#include <cstdint>
#include <optional>


namespace will::domain::matter {


class Underway {
public:
	Underway(id::Word training, std::uint32_t exercise, std::uint32_t approach, Timestamp begun,
			 std::optional<Timestamp> ended = std::nullopt);

	id::Word training() const noexcept { return training_; }
	std::uint32_t exercise() const noexcept { return exercise_; }
	std::uint32_t approach() const noexcept { return approach_; }
	Timestamp begun() const noexcept { return begun_; }
	const std::optional<Timestamp>& ended() const noexcept { return ended_; }

private:
	id::Word training_;
	std::uint32_t exercise_;
	std::uint32_t approach_;
	Timestamp begun_;
	std::optional<Timestamp> ended_;
};


} // namespace will::domain::matter
