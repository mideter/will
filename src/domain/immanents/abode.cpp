#include "abode.h"

#include "immanents/contemplation.h"
#include "immanents/man.h"
#include "immanents/witness.h"
#include "words/letter.h"
#include "matter/letter.h"
#include "horizons/space.h"
#include "properties/immanent.h"

#include <stdexcept>
#include <unordered_map>
#include <utility>


namespace will::domain {


Abode::Abode(const id::Abode id, AbodeName name)
	: Place(id::Place{id.value()})
	, name_(std::move(name))
	, mutex_(std::make_unique<std::mutex>())
{
	Immanent<Space>::present<Abode>();
}


Abode::Abode(Birth<Man>, matter::Abode kept)
	: Abode(kept.id(), kept.name())
{}


void Abode::admit(const Man& man)
{
	std::lock_guard lock(*mutex_);
	dwellers_.insert(&man);
}


bool Abode::dwells(const Man& man) const
{
	std::lock_guard lock(*mutex_);
	return dwellers_.contains(&man);
}


std::vector<std::shared_ptr<const Letter>> Abode::letters(const Contemplation& gaze) const
{
	if (&gaze.abode() != this)
		throw std::logic_error("this abode is not what is contemplated");

	std::lock_guard lock(*mutex_);

	bool whole = false;
	std::unordered_map<id::Word, std::shared_ptr<const Letter>> alive;
	std::vector<std::shared_ptr<const Letter>> shown;
	for (const std::shared_ptr<const Word>& word : living_words(whole)) {
		auto letter = std::dynamic_pointer_cast<const Letter>(word);
		if (!letter) {
			whole = false;
			continue;
		}

		alive.emplace(letter->id(), letter);
		shown.push_back(std::move(letter));
	}

	if (whole)
		return shown;

	shown.clear();
	for (matter::Letter& kept : kept_letters()) {
		const auto still = alive.find(kept.id());
		if (still != alive.end()) {
			shown.push_back(still->second);
			continue;
		}

		shown.push_back(std::make_shared<const Letter>(Birth<Abode>{}, std::move(kept)));
	}

	remember(std::vector<std::shared_ptr<const Word>>(shown.begin(), shown.end()));
	return shown;
}


std::shared_ptr<const Letter> Abode::inscribe(const Contemplation& gaze, matter::Letter kept) const
{
	if (&gaze.abode() != this)
		throw std::logic_error("this abode is not what is contemplated");
	if (kept.placement().place() != id())
		throw std::logic_error("the letter is not placed in this abode");
	if (kept.word().author() != gaze.who().Soul::id())
		throw std::logic_error("only its author inscribes a letter");

	auto letter = std::make_shared<const Letter>(Birth<Abode>{}, std::move(kept));

	std::lock_guard lock(*mutex_);
	remember(letter);

	return letter;
}


} // namespace will::domain
