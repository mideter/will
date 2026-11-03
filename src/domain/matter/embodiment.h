#pragma once

#include "identity/soul.h"
#include "identity/vessel.h"


namespace will::domain::matter {


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
