#pragma once

#include "relations/neighbour.h"


namespace will::domain {


/// Friend (Друг) — a neighbour who enters the inner part too.
class Friend : public Neighbour {
public:
	Friend(Birth<Abode> birth, const Man& man);

	using Neighbour::enters;
	bool enters(const Inner& part) const override;
};


} // namespace will::domain
