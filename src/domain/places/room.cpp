#include "room.h"
#include "refusals/room.h"
#include "refusals/abode.h"
#include "refusals/place.h"
#include "refusals/tie.h"
#include "refusals/testator.h"
#include "refusals/novice.h"
#include "refusals/training.h"
#include "refusals/refusal.h"

#include "horizons/heaven.h"
#include "horizons/space.h"
#include "men/man.h"
#include "men/novice.h"
#include "words/behest.h"
#include "words/deed.h"
#include "men/testator.h"
#include "men/witness.h"
#include "places/abode.h"
#include "places/tie.h"
#include "relations/acquaintance.h"
#include "relations/contemplation.h"
#include "words/training.h"

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


bool Cell::writable() const
{
	return reflects().takes_words_of(abode().host());
}


std::shared_ptr<const Word> Cell::write(const Contemplation& gaze, const Saying& saying) const
{
	if (&gaze.place() != this)
		throw std::logic_error("this cell is not what is contemplated");

	return reflects().receive(Birth<Cell>{*this}, saying);
}


std::shared_ptr<const Training> Cell::train(const Contemplation& gaze, const Saying& title,
											std::vector<Exercise> exercises) const
{
	if (&gaze.place() != this)
		throw std::logic_error("this cell is not what is contemplated");
	const auto* tie = dynamic_cast<const Tie*>(&reflects());
	if (!tie)
		throw WillOnlyInContemplatedTie();

	return tie->train(Birth<Cell>{*this}, title, std::move(exercises));
}


template<typename OtherTie, typename NotNovice>
const Novice& Cell::obeying(const Contemplation& gaze, const Behest& behest) const
{
	if (&gaze.place() != this)
		throw std::logic_error("this cell is not what is contemplated");
	if (&reflects() != static_cast<const Place*>(&behest.tie()))
		throw OtherTie();

	const Novice& novice = behest.tie().novice();
	if (abode().host().Soul::id() != novice.Soul::id())
		throw NotNovice();
	return novice;
}


std::shared_ptr<const Deed> Cell::fulfil(const Contemplation& gaze, const Behest& behest, std::optional<Saying> report,
										 std::optional<std::vector<Exercise>> performed) const
{
	return obeying<FulfilOnlyInContemplatedTie, OnlyNoviceFulfils>(gaze, behest)
		.execute(Birth<Cell>{*this}, behest, std::move(report), std::move(performed));
}


void Cell::begin(const Contemplation& gaze, const Training& training, const std::uint32_t exercise,
				 const std::uint32_t approach) const
{
	obeying<TrainOnlyInContemplatedTie, OnlyNoviceTrains>(gaze, training).begin(Birth<Cell>{*this}, training, exercise, approach);
}


matter::Effort Cell::finish(const Contemplation& gaze, const Training& training, const Weight weight,
							const std::uint32_t repetitions) const
{
	return obeying<TrainOnlyInContemplatedTie, OnlyNoviceTrains>(gaze, training).finish(Birth<Cell>{*this}, training, weight, repetitions);
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


void Gates::admit(const Contemplation& gaze, const Man& man) const
{
	if (&gaze.place() != this)
		throw std::logic_error("these gates are not what is contemplated");
	if (!keeps(gaze.who()))
		throw KeepGatesByKind();
	if (abode().dwells(man))
		throw AlreadyDwells();
	if (!static_cast<const Witness&>(man).contemplates(*this))
		throw NotAtGates();

	abode().host().admit(Birth<Gates>{*this}, man);
}


UpperRoom::UpperRoom(const Birth<Abode> birth, matter::Room kept)
	: Room(birth, birth.parent(), std::move(kept))
{}


std::string UpperRoom::name() const
{
	return "Горница";
}


void UpperRoom::regard(const Contemplation& gaze, const Man& dweller, const matter::Dweller::Kind kind) const
{
	if (&gaze.place() != this)
		throw std::logic_error("this upper room is not what is contemplated");
	if (&gaze.who() == &dweller)
		throw RegardNotOneself();

	abode().host().regard(Birth<UpperRoom>{*this}, dweller, kind);
}


void UpperRoom::arrange(const Contemplation& gaze, const Room& room, const matter::Room::Part part) const
{
	if (&gaze.place() != this)
		throw std::logic_error("this upper room is not what is contemplated");

	abode().host().arrange(Birth<UpperRoom>{*this}, room, part);
}


} // namespace will::domain
