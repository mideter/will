#pragma once

#include "entities/world.h"
#include "ports/temporality.h"


namespace will::domain {


/// Creation (Творение) — genesis: brings forth Heaven and Earth, the World,
/// indexes the World-abode on Earth, awakens the living cosmos, and rolls them
/// up when creation ends.
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
