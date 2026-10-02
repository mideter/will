#pragma once

#include "matter/dating.h"
#include "matter/execution.h"
#include "matter/placement.h"
#include "matter/utterance.h"
#include "beings/immanents/tie.h"
#include "beings/word.h"
#include "identity/word.h"
#include "values/saying.h"
#include "values/timestamp.h"

#include <optional>


namespace will::domain {


/// Deed (Дело) — Word of will in living Tie (Узы) that may be executed.
/// Born from three faces like Letter; executed once its matter::Execution is kept.
/// Sides are the Tie's: the testator utters, the novice executes.
class Deed : public Word {
public:
	/// Ids must match; place must be a living Tie whose testator is the author.
	Deed(matter::Utterance utterance, matter::Placement placement, matter::Dating dating,
		 std::optional<matter::Execution> execution = std::nullopt);

	id::Word id() const noexcept { return id_; }
	const Tie& tie() const noexcept { return tie_; }
	const Saying& saying() const noexcept { return saying_; }
	Timestamp created_at() const noexcept { return created_at_; }
	const std::optional<Timestamp>& executed_at() const noexcept { return executed_at_; }

	bool executed() const noexcept { return executed_at_.has_value(); }
	bool open() const noexcept { return !executed(); }

private:
	id::Word id_;
	const Tie& tie_;
	Saying saying_;
	Timestamp created_at_;
	std::optional<Timestamp> executed_at_;
};


} // namespace will::domain
