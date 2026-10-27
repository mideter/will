#include "tie.h"
#include "places/abode.h"
#include "places/room.h"
#include "refusals/tie.h"

#include "words/behest.h"
#include "words/deed.h"
#include "words/training.h"
#include "words/recollection.h"
#include "relations/contemplation.h"
#include "men/soul.h"
#include "horizons/space.h"
#include "men/man.h"
#include "men/testator.h"
#include "matter/behest.h"
#include "properties/immanent.h"

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>


namespace will::domain {


Tie::Tie(Birth<Novice>, matter::Tie kept)
	: Place(kept.id())
	, Obedience(Man::of(kept.testator()))
	, Shepherding(Man::of(kept.novice()))
{
	Immanent<Space>::present<Place>();
}


Tie::~Tie()
{
	testator_.release(*this);
}


const Man& Tie::testator() const
{
	return testator_;
}


const Man& Tie::novice() const
{
	return novice_;
}


namespace {


template<typename Kind>
std::vector<std::shared_ptr<const Kind>> of_kind(const std::vector<std::shared_ptr<const Word>>& words)
{
	std::vector<std::shared_ptr<const Kind>> shown;
	for (const std::shared_ptr<const Word>& word : words) {
		if (auto kind = std::dynamic_pointer_cast<const Kind>(word))
			shown.push_back(std::move(kind));
	}
	return shown;
}


} // namespace


std::vector<std::shared_ptr<const Behest>> Tie::behests(const Contemplation& gaze) const
{
	return of_kind<Behest>(words(gaze));
}


std::vector<std::shared_ptr<const Deed>> Tie::deeds(const Contemplation& gaze) const
{
	return of_kind<Deed>(words(gaze));
}


std::vector<std::shared_ptr<const Word>> Tie::words(const Contemplation& gaze) const
{
	const Room& room = gaze.room();
	if (&room.reflects() != static_cast<const Place*>(this))
		throw std::logic_error("these words are not what is contemplated");

	return room.words(gaze);
}


std::vector<std::shared_ptr<const Behest>> Tie::behests(const Birth<Abode> abode) const
{
	if (!dwells(abode.parent().host()))
		throw std::logic_error("the abode is not of a side of this tie");

	// Unbeheld, only behests are given: a behest holds its deed, while a deed knows its
	// behest only while a gaze holds them both.
	return of_kind<Behest>(recollection()->words());
}


std::shared_ptr<const Deed> Tie::inscribe(Birth<Novice>, const Behest& behest, matter::Deed kept) const
{
	if (&behest.tie() != this)
		throw std::logic_error("the behest is not of this tie");
	if (kept.placement().place() != id())
		throw std::logic_error("the deed is not placed in this tie");
	if (kept.word().author() != novice_.Soul::id())
		throw std::logic_error("only the novice of the tie inscribes a deed");

	auto deed = std::make_shared<const Deed>(Birth<Tie>{*this}, behest, std::move(kept));
	behest.deed(Birth<Tie>{*this}, deed);
	enter(*recollection(), deed);

	return deed;
}


std::shared_ptr<const Behest> Tie::inscribe(Birth<Testator>, matter::Behest kept) const
{
	if (kept.placement().place() != id())
		throw std::logic_error("the behest is not placed in this tie");
	if (kept.word().author() != testator_.Soul::id())
		throw std::logic_error("only the testator of the tie inscribes a behest");

	std::shared_ptr<const Behest> behest;
	if (kept.training())
		behest = std::make_shared<const Training>(Birth<Tie>{*this}, std::move(kept));
	else
		behest = std::make_shared<const Behest>(Birth<Tie>{*this}, std::move(kept));
	enter(*recollection(), behest);

	return behest;
}


bool Tie::dwells(const Man& man) const
{
	return man.Soul::id() == testator_.Soul::id() || man.Soul::id() == novice_.Soul::id();
}


const Man& Tie::counterpart(const Man& side) const
{
	if (testator().Soul::id() == side.Soul::id())
		return novice();
	return testator();
}


bool Tie::takes_words_of(const Man& host) const
{
	return host.Soul::id() == testator().Soul::id();
}


const Testator& Tie::testator_of(const Birth<Room> room) const
{
	if (&room.parent().reflects() != static_cast<const Place*>(this))
		throw std::logic_error("the room does not reflect this tie");
	if (!takes_words_of(room.parent().abode().host()))
		throw OnlyTestatorWills();
	return testator_;
}


std::shared_ptr<const Word> Tie::receive(const Birth<Room> room, const Saying& saying) const
{
	// What is written in its testator's room, he wills: a behest.
	return testator_of(room).will(room, *this, saying);
}


std::shared_ptr<const Training> Tie::train(const Birth<Room> room, const Saying& title,
										   std::vector<Exercise> exercises) const
{
	return testator_of(room).train(room, *this, title, std::move(exercises));
}


} // namespace will::domain
