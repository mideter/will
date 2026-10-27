#include "room.h"
#include "refusals/tie.h"
#include "refusals/testator.h"
#include "refusals/novice.h"
#include "refusals/training.h"

#include "horizons/space.h"
#include "men/man.h"
#include "men/novice.h"
#include "words/behest.h"
#include "words/deed.h"
#include "men/testator.h"
#include "places/abode.h"
#include "places/part.h"
#include "places/tie.h"
#include "relations/contemplation.h"
#include "words/recollection.h"
#include "words/training.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Room::Room(const Birth<Abode> birth, const Place& reflects, const Part& part, matter::Room kept)
	: Place(kept.id())
	, abode_(birth.parent())
	, reflects_(reflects)
	, part_(&part)
{
	if (kept.abode() != abode_.id())
		throw std::logic_error("the room is not of this abode");
	if (kept.reflects() != reflects_.id())
		throw std::logic_error("the room reflects another place");
	if (&part.abode() != &abode_)
		throw std::logic_error("the part is not of this abode");

	Immanent<Space>::present<Place>();
}


bool Room::dwells(const Man& man) const
{
	return part().admits(man);
}


bool Room::leads_to(const Man& who, const Place& next) const
{
	return abode_.leads_to(who, next);
}


bool Room::within(const Abode& abode) const
{
	return &abode == &abode_;
}


void Room::arrange(const Birth<Abode> birth, const Part& part) const
{
	if (&birth.parent() != &abode_ || &part.abode() != &abode())
		throw std::logic_error("only its abode arranges a room, in its own parts");

	part_.store(&part);
}


std::shared_ptr<const Recollection> Room::recollection(Birth<Contemplation>) const
{
	return reflects_.recollection(Birth<Room>{*this});
}


std::vector<std::shared_ptr<const Word>> Room::words(const Contemplation& gaze) const
{
	if (&gaze.place() != this)
		throw std::logic_error("this room is not what is contemplated");
	// One whose part no longer admits him, not yet withdrawn, sees nothing here.
	if (!dwells(gaze.who()))
		return {};

	return reflects_.recollection(Birth<Room>{*this})->words();
}


bool Room::writable_by(const Man& writer) const
{
	return &writer == &abode_.host() && reflects().takes_words_of(writer);
}


std::shared_ptr<const Word> Room::write(const Contemplation& gaze, const Saying& saying) const
{
	abode_.host_only(gaze, *this);

	return reflects().receive(Birth<Room>{*this}, saying);
}


std::shared_ptr<const Training> Room::train(const Contemplation& gaze, const Saying& title,
											std::vector<Exercise> exercises) const
{
	abode_.host_only(gaze, *this);
	const auto* tie = dynamic_cast<const Tie*>(&reflects());
	if (!tie)
		throw WillOnlyInContemplatedTie();

	return tie->train(Birth<Room>{*this}, title, std::move(exercises));
}


template<typename OtherTie, typename NotNovice>
const Novice& Room::obeying(const Contemplation& gaze, const Behest& behest) const
{
	abode_.host_only(gaze, *this);
	if (&reflects() != static_cast<const Place*>(&behest.tie()))
		throw OtherTie();

	const Novice& novice = behest.tie().novice();
	if (abode_.host().Soul::id() != novice.Soul::id())
		throw NotNovice();
	return novice;
}


std::shared_ptr<const Deed> Room::fulfil(const Contemplation& gaze, const Behest& behest, std::optional<Saying> report,
										 std::optional<std::vector<Exercise>> performed) const
{
	return obeying<FulfilOnlyInContemplatedTie, OnlyNoviceFulfils>(gaze, behest)
		.execute(Birth<Room>{*this}, behest, std::move(report), std::move(performed));
}


void Room::begin(const Contemplation& gaze, const Training& training, const std::uint32_t exercise,
				 const std::uint32_t approach) const
{
	obeying<TrainOnlyInContemplatedTie, OnlyNoviceTrains>(gaze, training).begin(Birth<Room>{*this}, training, exercise, approach);
}


void Room::end(const Contemplation& gaze, const Training& training) const
{
	obeying<TrainOnlyInContemplatedTie, OnlyNoviceTrains>(gaze, training).end(Birth<Room>{*this}, training);
}


matter::Effort Room::finish(const Contemplation& gaze, const Training& training, const Weight weight,
							const std::uint32_t repetitions, const Strain strain) const
{
	return obeying<TrainOnlyInContemplatedTie, OnlyNoviceTrains>(gaze, training).finish(Birth<Room>{*this}, training, weight, repetitions, strain);
}


matter::Remark Room::remark(const Contemplation& gaze, const Training& training, const std::uint32_t exercise,
							const Saying& saying) const
{
	return obeying<TrainOnlyInContemplatedTie, OnlyNoviceTrains>(gaze, training).remark(Birth<Room>{*this}, training, exercise, saying);
}


} // namespace will::domain
