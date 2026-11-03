#pragma once

#include "relations/neighbour.h"


namespace will::domain {


class Friend : public Neighbour {
public:
	Friend(Birth<OuterAbode> birth, const Man& man);

	using Neighbour::sees;
	bool sees(const Abode& abode) const override;
};


} // namespace will::domain
