#pragma once

#include "relations/acquaintance.h"


namespace will::domain {


/// Neighbour (Ближний) — an acquaintance who enters the outer part.
class Neighbour : public Acquaintance {
public:
	Neighbour(Birth<Abode> birth, const Man& man);

	using Acquaintance::enters;
	bool enters(const Outer& part) const override;
};


} // namespace will::domain
