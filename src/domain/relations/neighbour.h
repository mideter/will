#pragma once

#include "relations/acquaintance.h"


namespace will::domain {


/// An acquaintance who sees the rooms set outward.
class Neighbour : public Acquaintance {
public:
	Neighbour(Birth<OuterAbode> birth, const Man& man);

	using Acquaintance::sees;
	bool sees(const OuterAbode& outer) const override;
};


} // namespace will::domain
