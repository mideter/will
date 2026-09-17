#pragma once

#include "beings/earth.h"


namespace will::domain {


class Temporality;
class Vessel;


/// Dust (Прах) — earthly foundation of the vessel; knows Temporality through the one Earth.
/// Living Man presents the vessel to Earth at birth.
class Dust {
public:
	Temporality& temporality() const { return Earth::the().temporality(); }

	bool operator==(const Dust&) const = default;

protected:
	/// Present this living vessel to Earth (heap-stable address). Used by Man at birth.
	void present() const;
};


} // namespace will::domain
