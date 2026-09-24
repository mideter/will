#pragma once

#include "identity/letter.h"
#include "identity/place.h"


namespace will::domain {


/// Placement — Word fixed in a Place; material for living Letter. Spatiality keeps Placements.
class Placement {
public:
	Placement(id::Letter id, id::Place place);

	id::Letter id() const noexcept { return id_; }
	id::Place place() const noexcept { return place_; }

private:
	id::Letter id_;
	id::Place place_;
};


} // namespace will::domain
