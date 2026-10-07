#pragma once

#include "identity/word.h"
#include "matter/dating.h"
#include "matter/execution.h"
#include "matter/placement.h"
#include "matter/training.h"
#include "matter/word.h"

#include <optional>


namespace will::domain::matter {


/// Matter of living Deed (Дело) — the novice's word fulfilling a behest: its parts
/// gathered from the three dimensions and the Execution naming the behest, and,
/// fulfilling a training, the exercises done. Composite: no single dimension keeps
/// it. The parts share the word id.
class Deed {
public:
	Deed(Word word, Placement placement, Dating dating, Execution execution,
		 std::optional<Training> performed = std::nullopt);

	id::Word id() const noexcept { return word_.id(); }
	const Word& word() const noexcept { return word_; }
	const Placement& placement() const noexcept { return placement_; }
	const Dating& dating() const noexcept { return dating_; }
	const Execution& execution() const noexcept { return execution_; }

	/// The exercises done, if it fulfils a training.
	const std::optional<Training>& performed() const noexcept { return performed_; }

private:
	Word word_;
	Placement placement_;
	Dating dating_;
	Execution execution_;
	std::optional<Training> performed_;
};


} // namespace will::domain::matter
