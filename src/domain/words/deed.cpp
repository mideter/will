#include "deed.h"

#include "immanents/novice.h"
#include "immanents/place.h"

#include <stdexcept>
#include <utility>


namespace will::domain {
namespace {


const Tie& living_tie(const id::Place id)
{
	const auto* const tie = dynamic_cast<const Tie*>(&Place::of(id));
	if (!tie)
		throw std::invalid_argument("deed must be placed in a tie");

	return *tie;
}


} // namespace


Deed::Deed(Birth<Tie>, matter::Deed kept)
	: Deed(std::move(kept))
{}


Deed::Deed(Birth<Life>, matter::Deed kept)
	: Deed(std::move(kept))
{}


Deed::Deed(matter::Deed kept)
	: Word(kept.id(), kept.word().saying())
	, tie_(living_tie(kept.placement().place()))
	, behest_(kept.execution().behest())
	, created_at_(kept.dating().created_at())
{
	if (kept.word().author() != tie_.novice().Soul::id())
		throw std::invalid_argument("deed must be uttered by the novice of its tie");
}


} // namespace will::domain
