#include "room.h"

#include "horizons/heaven.h"
#include "horizons/space.h"
#include "men/man.h"
#include "men/testator.h"
#include "men/witness.h"
#include "places/abode.h"
#include "places/tie.h"
#include "relations/acquaintance.h"

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


void Room::arrange(const Birth<Abode> birth, const matter::Room& kept) const
{
	if (&birth.parent() != &abode_ || kept.id() != id())
		throw std::logic_error("only its abode arranges a room");

	part_.store(kept.part());
}


const Place& Room::source() const
{
	return reflects_.source();
}


bool Room::shows(const Man& who, const Word&) const
{
	return dwells(who);
}


bool Room::dwells(const Man& man) const
{
	if (&man == &abode_.host())
		return true;

	const std::shared_ptr<const Acquaintance> dweller = abode_.dweller(man);
	return dweller && dweller->enters(part());
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
		throw std::logic_error("one admits only standing in gates");
	if (!keeps(who))
		throw std::logic_error("one keeps the gates only by one's kind");
	if (abode().dwells(man))
		throw std::logic_error("he already dwells in this abode");
	if (!static_cast<const Witness&>(man).contemplates(*this))
		throw std::logic_error("he does not stand at the gates");

	abode().host().abode().admit(Birth<Gates>{*this}, man, kept);
}


const Place& Gates::source() const
{
	return Place::source();
}


bool Gates::shows(const Man&, const Word&) const
{
	return false;
}


UpperRoom::UpperRoom(const Birth<Abode> birth, matter::Room kept)
	: Room(birth, birth.parent(), std::move(kept))
{}


std::string UpperRoom::name() const
{
	return "Горница";
}


const Place& UpperRoom::source() const
{
	return Place::source();
}


bool UpperRoom::shows(const Man&, const Word&) const
{
	return false;
}


} // namespace will::domain
