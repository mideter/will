#pragma once

#include "beings/earth.h"
#include "properties/immanent.h"


namespace will::domain {


class Temporality;
class Spatiality;


/// Dust (Прах) — earthly foundation of the vessel; immanent to Earth.
/// Carrier of immanence: through Dust the vessel participates in Earth.
/// Knows Temporality and Spatiality through Earth.
class Dust : public Immanent<Earth> {
public:
	Temporality& temporality() const { return Earth::the().temporality(); }
	Spatiality& spatiality() const { return Earth::the().spatiality(); }

	bool operator==(const Dust&) const = default;
};


} // namespace will::domain
