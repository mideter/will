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


/// Creation (Творение) — genesis, the act of Life: brings forth Heaven, Earth,
/// the World, and awakens the living cosmos. Time and Space are of Eternity;
/// World is Heaven and Earth. Only Life creates; Creation is immanent to Life.
/// Spatiality and Temporality arise in Creation: it asks Eternity, reached
/// through the Life that gives it birth, to realise them, owns them, and gives
/// them to the World. One Creation in Life; their matter outlives it.
class Creation : public Immanent<Life> {
public:
	explicit Creation(Birth<Life> birth);

	/// The dimensions this Creation brought forth; they keep for whoever reads.
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
