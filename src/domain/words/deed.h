#pragma once

#include "identity/word.h"
#include "matter/deed.h"
#include "properties/birth.h"
#include "immanents/tie.h"
#include "words/word.h"
#include "values/timestamp.h"


namespace will::domain {


/// Deed (Дело) — the novice's Word in a living Tie (Узы) fulfilling a Behest: his
/// report, or «совершено» when he gives none. Born from matter::Deed; it does not
/// change. It knows the behest it fulfils.
class Deed : public Word {
public:
	/// Place must be a living Tie whose novice is the author.
	Deed(Birth<Tie>, matter::Deed kept);

	const Tie& tie() const noexcept { return tie_; }
	id::Word behest() const noexcept { return behest_; }
	Timestamp created_at() const noexcept { return created_at_; }

private:
	const Tie& tie_;
	id::Word behest_;
	Timestamp created_at_;
};


} // namespace will::domain
