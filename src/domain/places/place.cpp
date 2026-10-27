#include "place.h"

#include "horizons/life.h"
#include "places/room.h"
#include "words/recollection.h"
#include "horizons/space.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Place::Place()
	: id_(id::Place{1})
{
	throw std::logic_error("Place default ctor is only for virtual-base faces");
}


Place::Place(const id::Place id) noexcept
	: id_(id)
{}


const Place& Place::of(const id::Place id)
{
	return Space::the().place(id);
}


bool Place::leads_to(const Man&, const Place&) const
{
	return false;
}


bool Place::within(const OuterAbode&) const
{
	return false;
}


std::shared_ptr<const Recollection> Place::recollection(const Birth<Room> room) const
{
	if (&room.parent().reflects() != this)
		throw std::logic_error("the room does not reflect this place");

	return recollection();
}


std::shared_ptr<const Recollection> Place::recollection() const
{
	return Life::recollection(Birth<Place>{*this});
}


void Place::enter(const Recollection& recollection, std::shared_ptr<const Word> word) const
{
	recollection.enter(Birth<Place>{*this}, std::move(word));
}


bool Place::takes_words_of(const Man&) const
{
	return true;
}


std::shared_ptr<const Word> Place::receive(Birth<Room>, const Saying&) const
{
	throw std::logic_error("this place takes no written words");
}


} // namespace will::domain
