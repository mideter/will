#pragma once

#include "matter/deed.h"
#include "properties/birth.h"
#include "immanents/tie.h"
#include "words/word.h"
#include "values/timestamp.h"

#include <optional>


namespace will::domain {


class Testator;


/// Deed (Дело) — Word of will in living Tie (Узы) that may be executed.
/// Born from matter::Deed; executed once its Execution is kept.
/// Sides are the Tie's: the testator utters, the novice executes.
class Deed : public Word {
public:
	/// Place must be a living Tie whose testator is the author.
	Deed(Birth<Tie>, matter::Deed kept);
	Deed(Birth<Testator>, matter::Deed kept);

	const Tie& tie() const noexcept { return tie_; }
	Timestamp created_at() const noexcept { return created_at_; }
	const std::optional<Timestamp>& executed_at() const noexcept { return executed_at_; }

	bool executed() const noexcept { return executed_at_.has_value(); }
	bool open() const noexcept { return !executed(); }

private:
	explicit Deed(matter::Deed kept);

	const Tie& tie_;
	Timestamp created_at_;
	std::optional<Timestamp> executed_at_;
};


} // namespace will::domain
