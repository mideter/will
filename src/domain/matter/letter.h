#pragma once

#include "identity/word.h"
#include "matter/dating.h"
#include "matter/placement.h"
#include "matter/utterance.h"


namespace will::domain::matter {


/// Matter of living Letter (Письмо) — the parts of a Word gathered from the three
/// dimensions: uttered in Eternity, placed in Spatiality, dated in Temporality.
/// Composite: no single dimension keeps it. The parts share the word id.
class Letter {
public:
	Letter(Utterance utterance, Placement placement, Dating dating);

	id::Word id() const noexcept { return utterance_.id(); }
	const Utterance& utterance() const noexcept { return utterance_; }
	const Placement& placement() const noexcept { return placement_; }
	const Dating& dating() const noexcept { return dating_; }

private:
	Utterance utterance_;
	Placement placement_;
	Dating dating_;
};


} // namespace will::domain::matter
