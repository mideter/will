#pragma once

#include "horizons/earth.h"
#include "properties/immanent.h"


namespace will::domain {


class Temporality;
class Spatiality;


/// Dust (Прах) — the earthly foundation of the body: through it the body is of Earth.
class Dust : public Immanent<Earth> {
public:
	Temporality& temporality() const { return earth().temporality(); }
	Spatiality& spatiality() const { return earth().spatiality(); }

private:
	static Earth& earth() { return Earth::the(); }
};


} // namespace will::domain
