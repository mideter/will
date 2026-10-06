#include "room.h"

#include "horizons/space.h"
#include "men/man.h"
#include "men/testator.h"
#include "places/abode.h"
#include "places/tie.h"

#include <stdexcept>


namespace will::domain {


Room::Room(const Birth<Abode> birth, const Place& reflects, matter::Room kept)
	: Place(kept.id())
	, abode_(birth.parent())
	, reflects_(reflects)
	, part_(kept.part())
{
	if (kept.abode() != abode_.id())
		throw std::logic_error("the room is not of this abode");
	if (kept.reflects() != reflects_.id())
		throw std::logic_error("the room reflects another place");

	Immanent<Space>::present<Place>();
}


std::string Room::name() const
{
	if (&reflects_ == static_cast<const Place*>(&abode_))
		return "Келья";

	if (const auto* tie = dynamic_cast<const Tie*>(&reflects_)) {
		if (tie->testator().Soul::id() == abode_.host().Soul::id())
			return "Ведение — " + std::string{tie->novice().name().text()};
		return "Послушание — " + std::string{tie->testator().name().text()};
	}

	throw std::logic_error("a room reflects its abode or a tie");
}


bool Room::dwells(const Man& man) const
{
	return &man == &abode_.host();
}


} // namespace will::domain
