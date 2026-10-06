#pragma once

#include "relations/neighbour.h"


namespace will::domain {


/// Friend (Друг) — a neighbour who enters all the rooms of the Abode, of its
/// inner part and of its outer one.
class Friend : public Neighbour {
public:
	Friend(Birth<Abode> birth, const Man& man);

	bool enters(matter::Room::Part part) const override;
};


} // namespace will::domain
