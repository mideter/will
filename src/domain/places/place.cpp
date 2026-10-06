#include "place.h"

#include "horizons/life.h"
#include "relations/contemplation.h"
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


const Place& Place::source() const
{
	return *this;
}


bool Place::shows(const Man&, const Word&) const
{
	return true;
}


std::shared_ptr<const Recollection> Place::recollection(const Contemplation& gaze) const
{
	if (&gaze.place().source() != &source())
		throw std::logic_error("these words are not what is contemplated");

	return recollection();
}


std::shared_ptr<const Recollection> Place::recollection() const
{
	return Life::recollection(Birth<Place>{source()});
}


void Place::enter(const Recollection& recollection, std::shared_ptr<const Word> word) const
{
	recollection.enter(Birth<Place>{*this}, std::move(word));
}


} // namespace will::domain
