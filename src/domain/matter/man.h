#pragma once

#include "matter/embodiment.h"
#include "matter/soul.h"
#include "matter/vessel.h"


namespace will::domain::matter {


/// Soul and body, gathered from Eternity and Temporality.
class Man {
public:
	Man(Soul soul, Vessel vessel, const Embodiment& embodiment);

	const Soul& soul() const noexcept { return soul_; }
	const Vessel& vessel() const noexcept { return vessel_; }

private:
	Soul soul_;
	Vessel vessel_;
};


} // namespace will::domain::matter
