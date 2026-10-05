#pragma once

#include "matter/behest.h"
#include "properties/birth.h"
#include "immanents/tie.h"
#include "words/word.h"
#include "values/timestamp.h"


namespace will::domain {


class Life;
/// Behest (Веление) — the testator's Word of will in a living Tie (Узы).
/// Born from matter::Behest; it does not change. It is fulfilled by a Deed the
/// novice brings forth in the same Tie.
class Behest : public Word {
public:
	/// Willed or done now, born of its tie; or recalled from memory, born of Life.
	/// Place must be a living Tie whose testator is the author.
	Behest(Birth<Tie>, matter::Behest kept);
	Behest(Birth<Life>, matter::Behest kept);

	const Tie& tie() const noexcept { return tie_; }
	Timestamp created_at() const noexcept { return created_at_; }

private:
	explicit Behest(matter::Behest kept);

	const Tie& tie_;
	Timestamp created_at_;
};


} // namespace will::domain
