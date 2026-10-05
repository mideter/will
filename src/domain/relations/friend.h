#pragma once

#include "relations/neighbour.h"


namespace will::domain {


/// Friend (Друг) — a neighbour who sees all of the Abode, its inner part and its
/// outer one. Until the Abode has parts, he sees as a neighbour does.
class Friend : public Neighbour {
public:
	Friend(Birth<Abode> birth, const Man& man);
};


} // namespace will::domain
