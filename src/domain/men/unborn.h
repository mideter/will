#pragma once

#include "men/vessel.h"
#include "matter/vessel.h"
#include "properties/birth.h"


namespace will::domain {


class World;


/// Unborn (Нерождённый) — a body awaiting its soul: it only waits, seen by the keepers
/// of every gates by the mark of its body.
class Unborn final : public Vessel {
public:
	Unborn(Birth<World>, matter::Vessel kept);
};


} // namespace will::domain
