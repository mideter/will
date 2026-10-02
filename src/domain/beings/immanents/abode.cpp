#include "abode.h"

#include "beings/immanents/contemplation.h"
#include "beings/immanents/man.h"
#include "beings/immanents/witness.h"
#include "beings/letter.h"
#include "matter/letter.h"
#include "beings/space.h"
#include "properties/immanent.h"

#include <stdexcept>
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


std::vector<Letter> Abode::letters(const Contemplation& gaze) const
{
	if (&gaze.abode() != this)
		throw std::logic_error("this abode is not what is contemplated");
	if (!dwells(gaze.who()))
		throw std::logic_error("Witness does not dwell in the contemplated abode");

	std::vector<Parts> kept = words(gaze.who());

	std::vector<Letter> shown;
	shown.reserve(kept.size());
	for (Parts& parts : kept) {
		shown.emplace_back(matter::Letter{std::move(parts.utterance), std::move(parts.placement),
										  std::move(parts.dating)});
	}

	return shown;
}


} // namespace will::domain
