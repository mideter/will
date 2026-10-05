#include "abode.h"

#include "relations/contemplation.h"
#include "men/man.h"
#include "men/witness.h"
#include "words/letter.h"
#include "words/recollection.h"
#include "matter/letter.h"
#include "horizons/space.h"
#include "properties/immanent.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Abode::Abode(const Birth<Man> birth, matter::Abode kept)
	: Place(kept.id())
	, host_(birth.parent())
	, name_(kept.name())
	, mutex_(std::make_unique<std::mutex>())
{
	Immanent<Space>::present<Abode>();
}


void Abode::admit(const Birth<Man> birth, const Man& man, const matter::Dweller& kept)
{
	if (&birth.parent() != &host_)
		throw std::logic_error("only the host admits into his abode");

	admit(man, kept);
}


void Abode::admit(Birth<World>, const Man& man, const matter::Dweller& kept)
{
	admit(man, kept);
}


void Abode::admit(const Man& man, const matter::Dweller& kept)
{
	if (kept.abode() != id())
		throw std::logic_error("the dweller is not of this abode");
	if (kept.soul() != man.Soul::id())
		throw std::logic_error("the dweller matter is of another soul");
	if (&man == &host_)
		throw std::logic_error("the host is not a dweller of his own abode");

	std::lock_guard lock(*mutex_);
	dwellers_.insert_or_assign(&man, kept.kind());
}


std::optional<matter::Dweller::Kind> Abode::kind(const Man& man) const
{
	std::lock_guard lock(*mutex_);
	const auto it = dwellers_.find(&man);
	if (it == dwellers_.end())
		return std::nullopt;

	return it->second;
}


bool Abode::dwells(const Man& man) const
{
	if (&man == &host_)
		return true;

	std::lock_guard lock(*mutex_);
	return dwellers_.contains(&man);
}


std::shared_ptr<const Letter> Abode::inscribe(const Contemplation& gaze, matter::Letter kept) const
{
	if (&gaze.place() != this)
		throw std::logic_error("this abode is not what is contemplated");
	if (kept.placement().place() != id())
		throw std::logic_error("the letter is not placed in this abode");
	if (kept.word().author() != gaze.who().Soul::id())
		throw std::logic_error("only its author inscribes a letter");
	if (kept.word().author() != host_.Soul::id())
		throw std::logic_error("only the host writes in his abode");

	auto letter = std::make_shared<const Letter>(Birth<Abode>{*this}, std::move(kept));
	enter(*recollection(gaze), letter);

	return letter;
}


} // namespace will::domain
