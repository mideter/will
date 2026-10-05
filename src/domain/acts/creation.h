#pragma once

#include "horizons/world.h"
#include "dimensions/eternity.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"
#include "properties/birth.h"
#include "properties/immanent.h"


namespace will::domain {


class Life;


/// Creation (Творение) — genesis, the act of Life: brings forth Heaven, Earth,
/// the World, and awakens the living cosmos. Time and Space are of Eternity;
/// World is Heaven and Earth. Only Life creates; Creation is immanent to Life.
class Creation : public Immanent<Life> {
public:
	Creation(Birth<Life>, Eternity& eternity, Spatiality& spatiality, Temporality& temporality);

	World& world() noexcept { return world_; }
	const World& world() const noexcept { return world_; }

private:
	World world_;
};


} // namespace will::domain
