#pragma once

#include "identity/word.h"
#include "identity/place.h"


namespace will::domain {


/// Placement — Word fixed in a Place; material for living Letter. Spatiality keeps Placements.
class Placement {
public:
	Placement(id::Word id, id::Place place);

	id::Word id() const noexcept { return id_; }
	id::Place place() const noexcept { return place_; }

private:
	id::Word id_;
	id::Place place_;
};


} // namespace will::domain
