#include "letter.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Letter::Letter(Utterance utterance, Placement placement, Dating dating)
	: id_(utterance.id())
	, place_(Place::of(placement.place()))
	, author_(Soul::of(utterance.author()))
	, saying_(utterance.saying())
	, created_at_(dating.created_at())
{
	if (utterance.id() != placement.id() || utterance.id() != dating.id())
		throw std::invalid_argument("letter projections must share id");
}


} // namespace will::domain
