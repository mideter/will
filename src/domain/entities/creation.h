#pragma once

#include "entities/world.h"
#include "ports/temporality.h"


namespace will::domain {


/// Creation (Творение) — genesis: raises Heaven and Earth, brings forth the World,
/// and lowers the poles when creation ends.
class Creation {
public:
	explicit Creation(Temporality& temporality);
	~Creation();

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
