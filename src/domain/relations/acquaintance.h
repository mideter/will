#pragma once

#include "matter/room.h"
#include "properties/birth.h"
#include "properties/immanent.h"


namespace will::domain {


class Abode;
class Man;
class Space;


/// Acquaintance (Знакомый) — every dweller is at least one: he enters none of the rooms
/// and only knows that he dwells there. Regarded anew, he is born anew of the new kind.
class Acquaintance : public Immanent<Space> {
public:
	Acquaintance(Birth<Abode>, const Man& man);
	virtual ~Acquaintance() = default;

	const Man& man() const noexcept { return man_; }

	virtual bool enters(matter::Room::Part part) const;

private:
	const Man& man_;
};


} // namespace will::domain
