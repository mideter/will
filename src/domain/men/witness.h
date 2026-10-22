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


/// A living man in the waking world: awake, he contemplates one
/// thing, his own abode unless he turns elsewhere, where the place he stands in leads;
/// asleep, nothing.
class Witness : public virtual Embodied {
public:
	void wake() const;

	void sleep() const;

	/// A tie is seen only through the room reflecting it.
	void contemplate(const Abode& abode) const;
	void contemplate(const Room& room) const;

	/// The place itself, or a room reflecting it.
	bool contemplates(const Place& place) const;

	/// A letter in one's abode, written in its cell.
	std::shared_ptr<const Letter> record(Birth<Cell> cell, const Saying& saying) const;

protected:
	Witness() {}

private:
	void pass(const Place& next) const;
};


} // namespace will::domain
