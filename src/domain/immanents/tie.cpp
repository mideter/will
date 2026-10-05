#include "tie.h"

#include "words/behest.h"
#include "words/deed.h"
#include "words/recollection.h"
#include "immanents/contemplation.h"
#include "immanents/soul.h"
#include "horizons/space.h"
#include "immanents/testator.h"
#include "matter/behest.h"
#include "properties/immanent.h"

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>


namespace will::domain {


Tie::Tie(Birth<Novice>, matter::Tie kept)
	: Place(id::Place{kept.id().value()})
	, Obedience(Soul::of(kept.testator()))
	, Shepherding(Soul::of(kept.novice()))
{
	Immanent<Space>::present<Place>();
}


Tie::~Tie()
{
	testator_.release(*this);
}


const Testator& Tie::testator() const
{
	return testator_;
}


const Novice& Tie::novice() const
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
		throw std::logic_error("only the novice of a tie does a deed in it");

	auto deed = std::make_shared<const Deed>(Birth<Tie>{*this}, std::move(kept));
	enter(*recollection(), deed);

	return deed;
}


std::shared_ptr<const Behest> Tie::inscribe(Birth<Testator>, matter::Behest kept) const
{
	if (kept.placement().place() != id())
		throw std::logic_error("the behest is not placed in this tie");
	if (kept.word().author() != testator_.Soul::id())
		throw std::logic_error("only the testator of a tie wills in it");

	auto behest = std::make_shared<const Behest>(Birth<Tie>{*this}, std::move(kept));
	enter(*recollection(), behest);

	return behest;
}


bool Tie::dwells(const Man& man) const
{
	return man.Soul::id() == testator_.Soul::id() || man.Soul::id() == novice_.Soul::id();
}


} // namespace will::domain
