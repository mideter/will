#pragma once

#include "identity/word.h"
#include "matter/dating.h"
#include "matter/placement.h"
#include "matter/word.h"


namespace will::domain::matter {


/// Gathered from the three dimensions; no single one keeps it.
class Letter {
public:
	Letter(Word word, Placement placement, Dating dating);

	id::Word id() const noexcept { return word_.id(); }
	const Word& word() const noexcept { return word_; }
	const Placement& placement() const noexcept { return placement_; }
	const Dating& dating() const noexcept { return dating_; }

private:
	Word word_;
	Placement placement_;
	Dating dating_;
};


} // namespace will::domain::matter
