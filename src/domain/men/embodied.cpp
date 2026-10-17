#include "embodied.h"


namespace will::domain {


Embodied::Embodied(const matter::Man& kept)
	: Soul(kept.soul())
	, Vessel(kept.vessel())
{}


Embodied::~Embodied() = default;


} // namespace will::domain
