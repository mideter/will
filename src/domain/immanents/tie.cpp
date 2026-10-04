#include "tie.h"

#include "words/behest.h"
#include "words/deed.h"
#include "immanents/contemplation.h"
#include "immanents/soul.h"
#include "horizons/space.h"
#include "immanents/testator.h"
#include "matter/behest.h"
#include "properties/immanent.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <unordered_map>
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
	for (const std::shared_ptr<const Word>& word : living()) {
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
	for (const std::shared_ptr<const Word>& word : living()) {
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

	auto deed = std::make_shared<const Deed>(Birth<Tie>{}, std::move(kept));

	std::lock_guard lock(mutex_);
	remember(deed);

	return deed;
}


std::shared_ptr<const Behest> Tie::inscribe(Birth<Testator>, matter::Behest kept) const
{
	if (kept.placement().place() != id())
		throw std::logic_error("the behest is not placed in this tie");
	if (kept.word().author() != testator_.Soul::id())
		throw std::logic_error("only the testator of a tie wills in it");

	auto behest = std::make_shared<const Behest>(Birth<Tie>{}, std::move(kept));

	std::lock_guard lock(mutex_);
	remember(behest);

	return behest;
}


bool Tie::dwells(const Man& man) const
{
	return man.Soul::id() == testator_.Soul::id() || man.Soul::id() == novice_.Soul::id();
}


std::vector<std::shared_ptr<const Word>> Tie::words(const Contemplation& gaze) const
{
	if (&gaze.place() != static_cast<const Place*>(this))
		throw std::logic_error("this tie is not what is contemplated");

	return living();
}


std::vector<std::shared_ptr<const Word>> Tie::living() const
{
	std::lock_guard lock(mutex_);

	bool whole = false;
	std::vector<std::shared_ptr<const Word>> shown = living_words(whole);
	if (whole)
		return shown;

	std::unordered_map<id::Word, std::shared_ptr<const Word>> alive;
	for (std::shared_ptr<const Word>& word : shown)
		alive.emplace(word->id(), std::move(word));

	// Behests and deeds as Life gives them, merged into one stream by time.
	struct Born {
		Timestamp at;
		std::shared_ptr<const Word> word;
	};
	std::vector<Born> born;
	for (matter::Behest& kept : kept_behests()) {
		const Timestamp at = kept.dating().created_at();
		const auto still = alive.find(kept.id());
		born.push_back({at, still != alive.end()
								? still->second
								: std::make_shared<const Behest>(Birth<Tie>{}, std::move(kept))});
	}
	for (matter::Deed& kept : kept_deeds()) {
		const Timestamp at = kept.dating().created_at();
		const auto still = alive.find(kept.id());
		born.push_back({at, still != alive.end()
								? still->second
								: std::make_shared<const Deed>(Birth<Tie>{}, std::move(kept))});
	}
	std::stable_sort(born.begin(), born.end(), [](const Born& a, const Born& b) {
		if (a.at.value() != b.at.value())
			return a.at.value() < b.at.value();
		return a.word->id() < b.word->id();
	});

	shown.clear();
	shown.reserve(born.size());
	for (Born& each : born)
		shown.push_back(std::move(each.word));

	remember(shown);
	return shown;
}


} // namespace will::domain
