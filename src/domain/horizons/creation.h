#pragma once

#include "horizons/world.h"
#include "dimensions/eternity.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"
#include "properties/birth.h"
#include "properties/immanent.h"

#include <memory>


namespace will::domain {


class Life;


class Creation : public Immanent<Life> {
public:
	explicit Creation(Birth<Life> birth);

	Spatiality& spatiality() const noexcept { return *spatiality_; }
	Temporality& temporality() const noexcept { return *temporality_; }

	World& world() noexcept { return world_; }
	const World& world() const noexcept { return world_; }

private:
	std::unique_ptr<Spatiality> spatiality_;
	std::unique_ptr<Temporality> temporality_;
	World world_;
};


} // namespace will::domain
