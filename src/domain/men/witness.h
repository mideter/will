#pragma once

#include "men/embodied.h"
#include "properties/birth.h"
#include "values/saying.h"

#include <memory>


namespace will::domain {


class Abode;
class Room;
class Gates;
class Helm;
class Place;
class Letter;


/// A living man in the waking world: awake, he contemplates one thing, his own abode unless
/// he turns elsewhere, where the place he stands in leads; asleep, nothing. A tie he sees
/// only through the room reflecting it. Standing in a room he no longer enters, as its abode
/// changes, he withdraws into the abode itself. He records a letter in his abode, written in
/// its room.
class Witness : public virtual Embodied {
public:
	void wake() const;

	void sleep() const;

	void contemplate(const Abode& abode) const;
	void contemplate(const Room& room) const;
	void contemplate(const Gates& gates) const;
	void contemplate(const Helm& helm) const;

	bool contemplates(const Place& place) const;

	void withdraw(Birth<Abode> abode) const;

	std::shared_ptr<const Letter> record(Birth<Room> room, const Saying& saying) const;

protected:
	Witness() {}

private:
	void enter(const Place& place) const;

	void pass(const Place& next) const;
};


} // namespace will::domain
