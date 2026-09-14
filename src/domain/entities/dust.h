#pragma once

#include "entities/earth.h"


namespace will::domain {


class Temporality;


/// Dust (Прах) — earthly foundation of the vessel; knows Temporality through the one Earth.
class Dust {
public:
	Temporality& temporality() const { return Earth::the().temporality_; }

	bool operator==(const Dust&) const = default;
};


} // namespace will::domain
