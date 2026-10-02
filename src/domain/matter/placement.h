#pragma once

#include "identity/word.h"
#include "identity/place.h"


namespace will::domain::matter {


/// Placement — Word fixed in a Place; matter for a living Word (Letter, Deed).
/// Spatiality keeps Placements.
class Placement {
public:
	Placement(id::Word id, id::Place place);

	id::Word id() const noexcept { return id_; }
	id::Place place() const noexcept { return place_; }

private:
	id::Word id_;
	id::Place place_;
};


} // namespace will::domain::matter
