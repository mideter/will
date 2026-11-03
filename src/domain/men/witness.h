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
