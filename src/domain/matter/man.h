#pragma once

#include "matter/embodiment.h"
#include "matter/soul.h"
#include "matter/vessel.h"


namespace will::domain::matter {


/// Matter of living Man (Человек) — soul and body gathered from two dimensions:
/// the Soul from Eternity, the Vessel and the Embodiment from Temporality.
/// Composite: no single dimension keeps it.
class Man {
public:
	/// The embodiment must join exactly this soul to exactly this vessel.
	Man(Soul soul, Vessel vessel, const Embodiment& embodiment);

	const Soul& soul() const noexcept { return soul_; }
	const Vessel& vessel() const noexcept { return vessel_; }

private:
	Soul soul_;
	Vessel vessel_;
};


} // namespace will::domain::matter
