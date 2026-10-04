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


Abode::Abode(matter::Abode kept)
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

	std::unordered_map<id::Word, std::shared_ptr<const Letter>> alive;
	for (const std::weak_ptr<const Letter>& remembered : living_) {
		if (std::shared_ptr<const Letter> letter = remembered.lock())
			alive.emplace(letter->id(), std::move(letter));
	}

	if (!living_.empty() && alive.size() == living_.size()) {
		std::vector<std::shared_ptr<const Letter>> shown;
		shown.reserve(living_.size());
		for (const std::weak_ptr<const Letter>& remembered : living_)
			shown.push_back(remembered.lock());
		return shown;
	}

	std::vector<std::shared_ptr<const Letter>> shown;
	for (Parts& parts : words(gaze.who())) {
		const auto still = alive.find(parts.word.id());
		if (still != alive.end()) {
			shown.push_back(still->second);
			continue;
		}

		shown.push_back(std::make_shared<const Letter>(matter::Letter{
			std::move(parts.word), std::move(parts.placement), std::move(parts.dating)}));
	}

	living_.assign(shown.begin(), shown.end());
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

	auto letter = std::make_shared<const Letter>(std::move(kept));

	std::lock_guard lock(*mutex_);
	living_.push_back(letter);

	return letter;
}


} // namespace will::domain
