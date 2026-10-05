#pragma once

#include "relations/acquaintance.h"


namespace will::domain {


/// Neighbour (Ближний) — an acquaintance who sees the outer part of the Abode;
/// until the Abode has parts, all its words. A Friend is a neighbour who sees all.
class Neighbour : public Acquaintance {
public:
	Neighbour(Birth<Abode> birth, const Man& man);

	bool beholds(const Word& word) const override;
};


} // namespace will::domain
