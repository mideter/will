#pragma once

#include "beings/world.h"
#include "ports/eternity.h"
#include "ports/spatiality.h"
#include "ports/temporality.h"


namespace will::domain {


/// Creation (Творение) — genesis: brings forth Heaven, Earth, the World,
/// and awakens the living cosmos. Time and Space are of Eternity; World is Heaven and Earth.
class Creation {
public:
	Creation(Eternity& eternity, Spatiality& spatiality, Temporality& temporality);
	~Creation() = default;

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
