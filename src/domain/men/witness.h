#pragma once

#include "men/embodied.h"
#include "properties/birth.h"
#include "values/saying.h"

#include <memory>


namespace will::domain {


class Abode;
class Cell;
class Place;
class Room;
class Letter;


/// A living man in the waking world: awake, he contemplates one thing, his own abode unless
/// he turns elsewhere, where the place he stands in leads; asleep, nothing. A tie he sees
/// only through the room reflecting it. Standing in a room he no longer enters, as its abode
/// changes, he withdraws into the abode itself. He records a letter in his abode, written in
/// its cell.
class Witness : public virtual Embodied {
public:
	void wake() const;

	void sleep() const;

	void contemplate(const Abode& abode) const;
	void contemplate(const Room& room) const;

	bool contemplates(const Place& place) const;

	void withdraw(Birth<Abode> abode) const;

	std::shared_ptr<const Letter> record(Birth<Cell> cell, const Saying& saying) const;

protected:
	Witness() {}

private:
	void pass(const Place& next) const;
};


} // namespace will::domain
