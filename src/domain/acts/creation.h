#pragma once

#include "beings/space.h"
#include "beings/world.h"
#include "ports/eternity.h"
#include "ports/spatiality.h"
#include "ports/temporality.h"


namespace will::domain {


/// Creation (Творение) — genesis: brings forth Space, Heaven, Earth, the World,
/// and awakens the living cosmos. Space is an environment; World is Heaven and Earth.
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
	Space space_;
	World world_;
};


} // namespace will::domain
