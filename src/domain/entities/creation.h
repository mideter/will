#pragma once

#include "entities/world.h"
#include "ports/temporality.h"


namespace will::domain {


/// Creation (Творение) — genesis: raises Heaven and Earth, then brings forth the World.
class Creation {
public:
	explicit Creation(Temporality& temporality);

	Creation(const Creation&) = delete;
	Creation& operator=(const Creation&) = delete;
	Creation(Creation&&) = delete;
	Creation& operator=(Creation&&) = delete;

	World& world() noexcept { return world_; }
	const World& world() const noexcept { return world_; }

private:
	World world_;
};


} // namespace will::domain
