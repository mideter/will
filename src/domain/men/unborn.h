#pragma once

#include "men/vessel.h"
#include "matter/vessel.h"
#include "properties/birth.h"


namespace will::domain {


class World;


class Unborn final : public Vessel {
public:
	Unborn(Birth<World>, matter::Vessel kept);
};


} // namespace will::domain
