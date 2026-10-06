#pragma once

#include "matter/room.h"
#include "properties/birth.h"
#include "properties/immanent.h"


namespace will::domain {


class Abode;
class Man;
class Space;


/// Acquaintance (Знакомый) — a man dwelling in another's Abode as its host
/// admitted him: every dweller is at least an acquaintance. He enters none of
/// its rooms, and only knows that he dwells there. Born and held by the Abode;
/// regarded anew, he is born anew of the new kind. A Neighbour is an
/// acquaintance who enters more.
class Acquaintance : public Immanent<Space> {
public:
	Acquaintance(Birth<Abode>, const Man& man);
	virtual ~Acquaintance() = default;

	const Man& man() const noexcept { return man_; }

	/// Whether he enters the rooms of this part of the abode.
	virtual bool enters(matter::Room::Part part) const;

private:
	const Man& man_;
};


} // namespace will::domain
