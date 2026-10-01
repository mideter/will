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


Deed::Deed(Utterance utterance, Placement placement, Dating dating, std::optional<Execution> execution)
	: id_(utterance.id())
	, tie_(living_tie(placement.place()))
	, saying_(utterance.saying())
	, created_at_(dating.created_at())
{
	if (utterance.id() != placement.id() || utterance.id() != dating.id())
		throw std::invalid_argument("deed projections must share id");
	if (execution && execution->id() != utterance.id())
		throw std::invalid_argument("deed projections must share id");
	if (utterance.author() != tie_.testator().Soul::id())
		throw std::invalid_argument("deed must be uttered by the testator of its tie");

	if (execution)
		executed_at_ = execution->executed_at();
}


} // namespace will::domain
