#pragma once

#include "identity/soul.h"
#include "identity/vessel.h"


namespace will::domain::matter {


/// Embodiment (Воплощение) — a soul dwells in a vessel as its body, kept in time.
/// Part of the matter of Man; it has no living pair of its own: the living Man
/// is the soul in the body. Temporality keeps Embodiments.
class Embodiment {
public:
	Embodiment(id::Soul soul, id::Vessel vessel);

	id::Soul soul() const noexcept { return soul_; }
	id::Vessel vessel() const noexcept { return vessel_; }

private:
	id::Soul soul_;
	id::Vessel vessel_;
};


} // namespace will::domain::matter
