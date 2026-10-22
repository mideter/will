#include "tie.h"
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


std::vector<std::shared_ptr<const Behest>> Tie::behests(const Novice& asker) const
{
	if (asker.Soul::id() != novice_.Soul::id() && asker.Soul::id() != testator_.Soul::id())
		throw std::logic_error("not a side of this tie");

	std::vector<std::shared_ptr<const Behest>> shown;
	for (const std::shared_ptr<const Word>& word : recollection()->words()) {
		if (auto behest = std::dynamic_pointer_cast<const Behest>(word))
			shown.push_back(std::move(behest));
	}

	return shown;
}


std::vector<std::shared_ptr<const Deed>> Tie::deeds(const Novice& asker) const
{
	if (asker.Soul::id() != novice_.Soul::id() && asker.Soul::id() != testator_.Soul::id())
		throw std::logic_error("not a side of this tie");

	std::vector<std::shared_ptr<const Deed>> shown;
	for (const std::shared_ptr<const Word>& word : recollection()->words()) {
		if (auto deed = std::dynamic_pointer_cast<const Deed>(word))
			shown.push_back(std::move(deed));
	}

	return shown;
}


std::shared_ptr<const Deed> Tie::inscribe(Birth<Novice>, matter::Deed kept) const
{
	if (kept.placement().place() != id())
		throw std::logic_error("the deed is not placed in this tie");
	if (kept.word().author() != novice_.Soul::id())
		throw std::logic_error("only the novice of the tie inscribes a deed");

	auto deed = std::make_shared<const Deed>(Birth<Tie>{*this}, std::move(kept));
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


const Testator& Tie::testator_of(const Birth<Cell> cell) const
{
	if (&cell.parent().reflects() != static_cast<const Place*>(this))
		throw std::logic_error("the cell does not reflect this tie");
	if (!takes_words_of(cell.parent().abode().host()))
		throw OnlyTestatorWills();
	return testator_;
}


std::shared_ptr<const Word> Tie::receive(const Birth<Cell> cell, const Saying& saying) const
{
	// What is written in its testator's cell, he wills: a behest.
	return testator_of(cell).will(cell, *this, saying);
}


std::shared_ptr<const Training> Tie::train(const Birth<Cell> cell, const Saying& title,
										   std::vector<Exercise> exercises) const
{
	return testator_of(cell).train(cell, *this, title, std::move(exercises));
}


} // namespace will::domain
