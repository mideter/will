#pragma once

#include "horizons/world.h"
#include "dimensions/eternity.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"


namespace will::domain {


class Life;


/// Creation (Творение) — genesis, the act of Life: brings forth Heaven, Earth,
/// the World, and awakens the living cosmos. Time and Space are of Eternity;
/// World is Heaven and Earth. Only Life creates.
class Creation {
public:
	~Creation() = default;

	Creation(const Creation&) = delete;
	Creation& operator=(const Creation&) = delete;
	Creation(Creation&&) = delete;
	Creation& operator=(Creation&&) = delete;

	World& world() noexcept { return world_; }
	const World& world() const noexcept { return world_; }

private:
	friend class Life;

	Creation(Eternity& eternity, Spatiality& spatiality, Temporality& temporality);

	World world_;
};


} // namespace will::domain
