#pragma once

#include "matter/behest.h"
#include "properties/birth.h"
#include "places/tie.h"
#include "words/word.h"
#include "values/timestamp.h"


namespace will::domain {


class Life;
/// Behest (Веление, в интерфейсе «Задание») — the testator's word of will in a tie,
/// fulfilled by the novice's deed.
class Behest : public Word {
public:
	Behest(Birth<Tie>, matter::Behest kept);
	Behest(Birth<Life>, matter::Behest kept);

	const Tie& tie() const noexcept { return tie_; }
	Timestamp created_at() const noexcept { return created_at_; }

	const Soul& author() const override;

private:
	explicit Behest(matter::Behest kept);

	const Tie& tie_;
	Timestamp created_at_;
};


} // namespace will::domain
