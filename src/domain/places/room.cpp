#include "room.h"
#include "refusals/room.h"
#include "refusals/abode.h"
#include "refusals/place.h"
#include "refusals/refusal.h"

#include "horizons/heaven.h"
#include "horizons/space.h"
#include "men/man.h"
#include "men/testator.h"
#include "men/witness.h"
#include "places/abode.h"
#include "places/tie.h"
#include "relations/acquaintance.h"
#include "relations/contemplation.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Room::Room(const Birth<Abode> birth, const Place& reflects, matter::Room kept)
	: Place(kept.id())
	, abode_(birth.parent())
	, reflects_(reflects)
	, aspect_(kept.aspect())
	, part_(kept.part())
{
	if (kept.abode() != abode_.id())
		throw std::logic_error("the room is not of this abode");
	if (kept.reflects() != reflects_.id())
		throw std::logic_error("the room reflects another place");

	Immanent<Space>::present<Place>();
}


void Room::arrange(const Birth<Abode> birth, const matter::Room& kept) const
{
	if (&birth.parent() != &abode_ || kept.id() != id())
		throw std::logic_error("only its abode arranges a room");

	part_.store(kept.part());
}


bool Room::dwells(const Man& man) const
{
	if (&man == &abode_.host())
		return true;

	const std::shared_ptr<const Acquaintance> dweller = abode_.dweller(man);
	return dweller && dweller->enters(part());
}


Cell::Cell(const Birth<Abode> birth, const Place& reflects, matter::Room kept)
	: Room(birth, reflects, std::move(kept))
{}


std::string Cell::name() const
{
	if (&reflects() == static_cast<const Place*>(&abode()))
		return "Слово";

	if (const auto* tie = dynamic_cast<const Tie*>(&reflects())) {
		if (tie->testator().Soul::id() == abode().host().Soul::id())
			return "Ведение — " + std::string{tie->novice().name().text()};
		return "Послушание — " + std::string{tie->testator().name().text()};
	}

	throw std::logic_error("a room reflects its abode or a tie");
}


const Place& Cell::source() const
{
	return reflects().source();
}


bool Cell::shows(const Man& who, const Word&) const
{
	return dwells(who);
}


bool Cell::writable_by(const Man& writer) const
{
	return &abode() == &writer.abode() && reflects().takes_words_of(writer);
}


std::shared_ptr<const Word> Cell::write(const Contemplation& gaze, const Saying& saying) const
{
	if (&gaze.place() != this)
		throw std::logic_error("this cell is not what is contemplated");
	if (&abode() != &gaze.who().abode())
		throw SayOnlyInWord();

	return reflects().receive(gaze, saying);
}


Gates::Gates(const Birth<Abode> birth, matter::Room kept)
	: Room(birth, birth.parent(), std::move(kept))
{}


std::string Gates::name() const
{
	return "Врата";
}


bool Gates::dwells(const Man&) const
{
	return true;
}


bool Gates::keeps(const Man& man) const
{
	return Room::dwells(man);
}


bool Gates::open() const
{
	for (const Soul& soul : Immanent<Heaven>::horizon().contemplating(*this)) {
		if (keeps(static_cast<const Man&>(soul)))
			return true;
	}
	return false;
}


void Gates::admit(const Birth<Man> keeper, const Man& man, const matter::Dweller& kept) const
{
	const Man& who = keeper.parent();
	if (!static_cast<const Witness&>(who).contemplates(*this))
		throw AdmitOnlyAtGates();
	if (!keeps(who))
		throw KeepGatesByKind();
	if (abode().dwells(man))
		throw AlreadyDwells();
	if (!static_cast<const Witness&>(man).contemplates(*this))
		throw NotAtGates();

	abode().host().abode().admit(Birth<Gates>{*this}, man, kept);
}


UpperRoom::UpperRoom(const Birth<Abode> birth, matter::Room kept)
	: Room(birth, birth.parent(), std::move(kept))
{}


std::string UpperRoom::name() const
{
	return "Горница";
}


} // namespace will::domain
