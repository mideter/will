#pragma once

#include "entities/earth.h"


namespace will::domain {


class Temporality;
class Vessel;


/// Dust (Прах) — earthly foundation of the vessel; knows Temporality through the one Earth.
/// Presents a living vessel to Earth.
class Dust {
public:
	Temporality& temporality() const { return Earth::the().temporality_; }

	/// Present this living vessel to Earth (heap-stable address).
	void present(const Vessel& vessel) const;

	bool operator==(const Dust&) const = default;
};


} // namespace will::domain
