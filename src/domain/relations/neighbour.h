#pragma once

#include "relations/acquaintance.h"


namespace will::domain {


/// Neighbour (Ближний) — an acquaintance who enters the rooms of the outer part
/// of the Abode. A Friend is a neighbour who enters all.
class Neighbour : public Acquaintance {
public:
	Neighbour(Birth<Abode> birth, const Man& man);

	bool enters(matter::Room::Part part) const override;
};


} // namespace will::domain
