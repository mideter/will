#include "letter.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Letter::Letter(Utterance utterance, Placement placement, Dating dating)
	: Word(utterance.word())
	, id_(utterance.id())
	, place_(Place::of(placement.place()))
	, author_(Soul::of(utterance.author()))
	, created_at_(dating.created_at())
{
	if (utterance.id() != placement.id() || utterance.id() != dating.id())
		throw std::invalid_argument("letter projections must share id");
}


} // namespace will::domain
