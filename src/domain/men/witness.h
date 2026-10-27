#pragma once

#include "men/embodied.h"
#include "properties/birth.h"
#include "values/saying.h"

#include <memory>


namespace will::domain {


class Abode;
class Gates;
class Helm;
class OuterAbode;
class Room;
class Place;
class Letter;


/// A living man in the waking world: awake, he contemplates one thing, his own abode unless
/// he turns elsewhere, where the place he stands in leads; asleep, nothing. He says who
/// stands where: anyone at gates; at a helm only its host; in a room one its abode shows it
/// to; in an abode one who dwells there, any other coming only to its gates. A tie he sees
/// only through the room reflecting it. Standing in a room his abode no longer shows him, he
/// withdraws into the abode itself. He records a letter in his abode, written in its room.
class Witness : public virtual Embodied {
public:
	void wake() const;

	void sleep() const;

	void contemplate(const Abode& abode) const;
	void contemplate(const Room& room) const;
	void contemplate(const Gates& gates) const;
	void contemplate(const Helm& helm) const;

	bool contemplates(const Place& place) const;

	void withdraw(Birth<OuterAbode> abode) const;

	std::shared_ptr<const Letter> record(Birth<Room> room, const Saying& saying) const;

protected:
	Witness() {}

private:
	void enter(const Place& place) const;
	void pass(const Place& next) const;
};


} // namespace will::domain
