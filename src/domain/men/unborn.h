#pragma once

#include "men/vessel.h"
#include "matter/vessel.h"
#include "properties/birth.h"


namespace will::domain {


class World;


/// Unborn (Нерождённый) — a body awaiting its soul: a device came into the world,
/// but no one has borne it yet. It only waits, seen in every birth room by the
/// mark of its body. Born and held by the World until it is born a man.
class Unborn final : public Vessel {
public:
	Unborn(Birth<World>, matter::Vessel kept);
};


} // namespace will::domain
