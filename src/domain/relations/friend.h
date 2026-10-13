#pragma once

#include "relations/neighbour.h"


namespace will::domain {


/// Friend (Друг) — a neighbour who enters all the rooms.
class Friend : public Neighbour {
public:
	Friend(Birth<Abode> birth, const Man& man);

	bool enters(matter::Room::Part part) const override;
};


} // namespace will::domain
