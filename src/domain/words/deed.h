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
/// The novice's word fulfilling a behest: his report, or «совершено».
class Deed : public Word {
public:
	Deed(Birth<Tie>, matter::Deed kept);
	Deed(Birth<Life>, matter::Deed kept);

	const Tie& tie() const noexcept { return tie_; }
	id::Word behest() const noexcept { return behest_; }

	const Soul& author() const override;

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
