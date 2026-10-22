#include "room.h"
#include "refusals/room.h"
#include "refusals/abode.h"
#include "refusals/tie.h"
#include "refusals/testator.h"
#include "refusals/novice.h"
#include "refusals/training.h"

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
{
	if (kept.abode() != abode_.id())
		throw std::logic_error("the room is not of this abode");
	if (kept.reflects() != reflects_.id())
		throw std::logic_error("the room reflects another place");

	Immanent<Space>::present<Place>();
}


bool Room::leads_to(const Place& next) const
{
	return abode_.leads_to(next);
}


bool Room::within(const Abode& abode) const
{
	return &abode == &abode_;
}


void Room::host_only(const Contemplation& gaze) const
{
	if (&gaze.place() != this)
		throw std::logic_error("this room is not what is contemplated");
	if (&gaze.who() != &abode().host())
		throw OnlyHostActs();
}


Cell::Cell(const Birth<Abode> birth, const Place& reflects, matter::Room kept)
	: Room(birth, reflects, kept)
	, part_(kept.part())
{}


bool Cell::dwells(const Man& man) const
{
	if (&man == &abode().host())
		return true;

	const std::shared_ptr<const Acquaintance> dweller = abode().dweller(man);
	return dweller && dweller->enters(part());
}


void Cell::arrange(const Birth<Abode> birth, const matter::Room& kept) const
{
	if (&birth.parent() != &abode() || kept.id() != id())
		throw std::logic_error("only its abode arranges a cell");

	part_.store(kept.part());
}


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
	return &writer == &abode().host() && reflects().takes_words_of(writer);
}


std::shared_ptr<const Word> Cell::write(const Contemplation& gaze, const Saying& saying) const
{
	host_only(gaze);

	return reflects().receive(Birth<Cell>{*this}, saying);
}


std::shared_ptr<const Training> Cell::train(const Contemplation& gaze, const Saying& title,
											std::vector<Exercise> exercises) const
{
	host_only(gaze);
	const auto* tie = dynamic_cast<const Tie*>(&reflects());
	if (!tie)
		throw WillOnlyInContemplatedTie();

	return tie->train(Birth<Cell>{*this}, title, std::move(exercises));
}


template<typename OtherTie, typename NotNovice>
const Novice& Cell::obeying(const Contemplation& gaze, const Behest& behest) const
{
	host_only(gaze);
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


bool Gates::leads_to(const Place& next) const
{
	return next.within(abode()) || dynamic_cast<const Gates*>(&next);
}


bool Gates::within(const Abode&) const
{
	return false;
}


bool Gates::keeps(const Man& man) const
{
	return &man == &abode().host();
}


bool Gates::open() const
{
	for (const Man& man : Immanent<Heaven>::horizon().contemplating(*this)) {
		if (keeps(man))
			return true;
	}
	return false;
}


void Gates::admit(const Contemplation& gaze, const Man& man) const
{
	host_only(gaze);
	if (abode().dwells(man))
		throw AlreadyDwells();
	if (!man.contemplates(*this))
		throw NotAtGates();

	abode().host().admit(Birth<Gates>{*this}, man);
}


const Man& Gates::bear(const Contemplation& gaze, const Unborn& unborn) const
{
	host_only(gaze);

	return Immanent<Heaven>::horizon().bear(Birth<Gates>{*this}, unborn);
}


UpperRoom::UpperRoom(const Birth<Abode> birth, matter::Room kept)
	: Room(birth, birth.parent(), std::move(kept))
{}


std::string UpperRoom::name() const
{
	return "Горница";
}


bool UpperRoom::dwells(const Man& man) const
{
	return &man == &abode().host();
}


void UpperRoom::regard(const Contemplation& gaze, const Man& dweller, const matter::Dweller::Kind kind) const
{
	host_only(gaze);

	abode().host().regard(Birth<UpperRoom>{*this}, dweller, kind);
}


void UpperRoom::supplicate(const Contemplation& gaze, const Man& addressee) const
{
	host_only(gaze);
	const std::shared_ptr<const Acquaintance> dweller = abode().dweller(addressee);
	if (!dweller)
		throw DoesNotDwell();

	abode().host().supplicate(Birth<UpperRoom>{*this}, *dweller);
}


void UpperRoom::arrange(const Contemplation& gaze, const Cell& cell, const matter::Room::Part part) const
{
	host_only(gaze);

	abode().host().arrange(Birth<UpperRoom>{*this}, cell, part);
}


} // namespace will::domain
