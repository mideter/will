#include "man.h"

#include <stdexcept>
#include <utility>


namespace will::domain::matter {


Man::Man(Soul soul, Vessel vessel, const Embodiment& embodiment)
	: soul_(std::move(soul))
	, vessel_(std::move(vessel))
{
	if (embodiment.soul() != soul_.id() || embodiment.vessel() != vessel_.id())
		throw std::invalid_argument("embodiment does not join this soul to this vessel");
}


} // namespace will::domain::matter
