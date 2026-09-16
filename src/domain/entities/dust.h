#pragma once

#include "entities/earth.h"


namespace will::domain {


class Temporality;
class Vessel;


/// Dust (Прах) — earthly foundation of the vessel; knows Temporality through the one Earth.
/// Indexes a living vessel into Earth.
class Dust {
public:
	Temporality& temporality() const { return Earth::the().temporality_; }

	/// Present this living vessel to Earth (heap-stable address).
	void index(const Vessel& vessel) const;

	bool operator==(const Dust&) const = default;
};


} // namespace will::domain
