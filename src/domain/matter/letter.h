#pragma once

#include "identity/word.h"
#include "matter/dating.h"
#include "matter/placement.h"
#include "matter/word.h"


namespace will::domain::matter {


/// Matter of living Letter (Письмо) — the parts of a Word gathered from the three
/// dimensions: uttered in Eternity, placed in Spatiality, dated in Temporality.
/// Composite: no single dimension keeps it. The parts share the word id.
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
