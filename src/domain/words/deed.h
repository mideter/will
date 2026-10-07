#pragma once

#include "identity/word.h"
#include "matter/deed.h"
#include "properties/birth.h"
#include "places/tie.h"
#include "words/word.h"
#include "values/exercise.h"
#include "values/timestamp.h"

#include <vector>


namespace will::domain {


class Life;
/// Deed (Дело) — the novice's Word in a living Tie (Узы) fulfilling a Behest: his
/// report, or «совершено» when he gives none. Born from matter::Deed; it does not
/// change. It knows the behest it fulfils and, for a training, what he has done.
class Deed : public Word {
public:
	/// Willed or done now, born of its tie; or recalled from memory, born of Life.
	/// Place must be a living Tie whose novice is the author.
	Deed(Birth<Tie>, matter::Deed kept);
	Deed(Birth<Life>, matter::Deed kept);

	const Tie& tie() const noexcept { return tie_; }
	id::Word behest() const noexcept { return behest_; }

	/// The exercises done, if it fulfils a training; none otherwise.
	const std::vector<Exercise>& performed() const noexcept { return performed_; }
	Timestamp created_at() const noexcept { return created_at_; }

private:
	explicit Deed(matter::Deed kept);

	const Tie& tie_;
	id::Word behest_;
	std::vector<Exercise> performed_;
	Timestamp created_at_;
};


} // namespace will::domain
