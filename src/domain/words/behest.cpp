#include "behest.h"

#include "words/deed.h"
#include "places/place.h"
#include "places/tie.h"
#include "men/man.h"

#include <stdexcept>
#include <utility>


namespace will::domain {
namespace {


const Tie& living_tie(const id::Place id)
{
	const auto* const tie = dynamic_cast<const Tie*>(&Place::of(id));
	if (!tie)
		throw std::invalid_argument("behest must be placed in a tie");

	return *tie;
}


} // namespace


Behest::Behest(Birth<Tie>, matter::Behest kept)
	: Behest(std::move(kept))
{}


Behest::Behest(Birth<Life>, matter::Behest kept)
	: Behest(std::move(kept))
{}


Behest::Behest(matter::Behest kept)
	: Word(kept.id(), kept.word().saying())
	, tie_(living_tie(kept.placement().place()))
	, created_at_(kept.dating().created_at())
{
	if (kept.word().author() != tie_.testator().Soul::id())
		throw std::invalid_argument("behest must be uttered by the testator of its tie");
}


const Soul& Behest::author() const
{
	return tie_.testator();
}


std::shared_ptr<const Deed> Behest::deed() const
{
	std::lock_guard lock(*mutex_);
	return deed_;
}


void Behest::deed(Birth<Tie>, std::shared_ptr<const Deed> deed) const
{
	fulfil(std::move(deed));
}


void Behest::deed(Birth<Life>, std::shared_ptr<const Deed> deed) const
{
	fulfil(std::move(deed));
}


void Behest::fulfil(std::shared_ptr<const Deed> deed) const
{
	if (!deed || &deed->behest() != this)
		throw std::logic_error("the deed fulfils another behest");

	std::lock_guard lock(*mutex_);
	if (deed_)
		throw std::logic_error("the behest is already fulfilled");
	deed_ = std::move(deed);
}


} // namespace will::domain
