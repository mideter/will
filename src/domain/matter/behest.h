#pragma once

#include "identity/word.h"
#include "matter/dating.h"
#include "matter/placement.h"
#include "matter/training.h"
#include "matter/word.h"

#include <optional>


namespace will::domain::matter {


/// Matter of living Behest (Веление) — the parts of a Word of will gathered from
/// the three dimensions, and, for a training, its exercises. Composite: no single
/// dimension keeps it. The parts share the word id.
class Behest {
public:
	Behest(Word word, Placement placement, Dating dating, std::optional<Training> training = std::nullopt);

	id::Word id() const noexcept { return word_.id(); }
	const Word& word() const noexcept { return word_; }
	const Placement& placement() const noexcept { return placement_; }
	const Dating& dating() const noexcept { return dating_; }

	/// The exercises it wills, if it wills a training.
	const std::optional<Training>& training() const noexcept { return training_; }

private:
	Word word_;
	Placement placement_;
	Dating dating_;
	std::optional<Training> training_;
};


} // namespace will::domain::matter
