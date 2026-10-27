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
#include "relations/knock.h"
#include "words/training.h"

#include <algorithm>
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


Cell::Cell(const Birth<Abode> birth, const Place& reflects, const Part& part, matter::Room kept)
	: Room(birth, reflects, std::move(kept))
	, part_(&part)
{
	if (&part.abode() != &abode())
		throw std::logic_error("the part is not of this abode");
}


bool Cell::dwells(const Man& man) const
{
	return part().admits(man);
}


void Cell::arrange(const Birth<Abode> birth, const Part& part) const
{
	if (&birth.parent() != &abode() || &part.abode() != &abode())
		throw std::logic_error("only its abode arranges a cell, in its own parts");

	part_.store(&part);
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


void Cell::end(const Contemplation& gaze, const Training& training) const
{
	obeying<TrainOnlyInContemplatedTie, OnlyNoviceTrains>(gaze, training).end(Birth<Cell>{*this}, training);
}


matter::Effort Cell::finish(const Contemplation& gaze, const Training& training, const Weight weight,
							const std::uint32_t repetitions, const Strain strain) const
{
	return obeying<TrainOnlyInContemplatedTie, OnlyNoviceTrains>(gaze, training).finish(Birth<Cell>{*this}, training, weight, repetitions, strain);
}


matter::Remark Cell::remark(const Contemplation& gaze, const Training& training, const std::uint32_t exercise,
							const Saying& saying) const
{
	return obeying<TrainOnlyInContemplatedTie, OnlyNoviceTrains>(gaze, training).remark(Birth<Cell>{*this}, training, exercise, saying);
}


Gates::Gates(const Birth<Abode> birth, matter::Room kept)
	: Room(birth, birth.parent(), std::move(kept))
{}


std::string Gates::name() const
{
	return "Врата";
}


const Part& Gates::part() const
{
	return abode().outer();
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


void Gates::knock(const Contemplation& gaze) const
{
	if (&gaze.place() != this)
		throw std::logic_error("these gates are not what is contemplated");
	if (abode().dwells(gaze.who()))
		throw AlreadyDwells();
	if (dynamic_cast<const Knock*>(&gaze))
		throw AlreadyKnocks();

	Immanent<Heaven>::horizon().knock(Birth<Gates>{*this}, gaze);
}


std::vector<std::reference_wrapper<const Man>> Gates::knocking() const
{
	// One let in while he knocks waits no longer.
	std::vector<std::reference_wrapper<const Man>> waiting;
	for (const Man& man : Immanent<Heaven>::horizon().knocking(*this)) {
		if (!abode().dwells(man))
			waiting.emplace_back(man);
	}
	return waiting;
}


void Gates::admit(const Contemplation& gaze, const Man& man) const
{
	host_only(gaze);
	if (abode().dwells(man))
		throw AlreadyDwells();
	const auto waiting = knocking();
	if (std::ranges::none_of(waiting, [&](const Man& knocker) { return &knocker == &man; }))
		throw DoesNotKnock();

	abode().host().admit(Birth<Gates>{*this}, man);
}


const Man& Gates::bear(const Contemplation& gaze, const Unborn& unborn) const
{
	host_only(gaze);

	return Immanent<Heaven>::horizon().bear(Birth<Gates>{*this}, unborn);
}


Helm::Helm(const Birth<Abode> birth, matter::Room kept)
	: Room(birth, birth.parent(), std::move(kept))
{}


std::string Helm::name() const
{
	return "Кормчая";
}


const Part& Helm::part() const
{
	return abode().inner();
}


bool Helm::dwells(const Man& man) const
{
	return &man == &abode().host();
}


void Helm::regard(const Contemplation& gaze, const Man& dweller, const matter::Dweller::Kind kind) const
{
	host_only(gaze);

	abode().host().regard(Birth<Helm>{*this}, dweller, kind);
}


void Helm::supplicate(const Contemplation& gaze, const Man& addressee) const
{
	host_only(gaze);
	const std::shared_ptr<const Acquaintance> dweller = abode().dweller(addressee);
	if (!dweller)
		throw DoesNotDwell();

	abode().host().supplicate(Birth<Helm>{*this}, *dweller);
}


const Shepherding& Helm::accept(const Contemplation& gaze, const Supplication& ask) const
{
	host_only(gaze);

	return abode().host().accept(Birth<Helm>{*this}, ask);
}


void Helm::reject(const Contemplation& gaze, const Supplication& ask) const
{
	host_only(gaze);

	abode().host().reject(Birth<Helm>{*this}, ask);
}


void Helm::arrange(const Contemplation& gaze, const Cell& cell, const Part& part) const
{
	host_only(gaze);

	abode().host().arrange(Birth<Helm>{*this}, cell, part);
}


} // namespace will::domain
