#pragma once

#include "relations/acquaintance.h"


namespace will::domain {


class Neighbour : public Acquaintance {
public:
	Neighbour(Birth<OuterAbode> birth, const Man& man);

	using Acquaintance::sees;
	bool sees(const OuterAbode& outer) const override;
};


} // namespace will::domain
