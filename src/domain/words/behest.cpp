#include "behest.h"

#include "immanents/place.h"
#include "immanents/testator.h"

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


Behest::Behest(Birth<Testator>, matter::Behest kept)
	: Behest(std::move(kept))
{}


Behest::Behest(matter::Behest kept)
	: Word(kept.id(), kept.word().saying())
	, tie_(living_tie(kept.placement().place()))
	, created_at_(kept.dating().created_at())
{
	if (kept.word().author() != tie_.testator().Soul::id())
		throw std::invalid_argument("behest must be uttered by the testator of its tie");

	if (kept.execution())
		executed_at_ = kept.execution()->executed_at();
}


} // namespace will::domain
