#include "deed.h"

#include "beings/immanents/place.h"
#include "beings/immanents/testator.h"

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


Deed::Deed(matter::Deed kept)
	: id_(kept.id())
	, tie_(living_tie(kept.placement().place()))
	, saying_(kept.utterance().saying())
	, created_at_(kept.dating().created_at())
{
	if (kept.utterance().author() != tie_.testator().Soul::id())
		throw std::invalid_argument("deed must be uttered by the testator of its tie");

	if (kept.execution())
		executed_at_ = kept.execution()->executed_at();
}


} // namespace will::domain
