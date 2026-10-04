#include "letter.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Letter::Letter(Birth<Abode>, matter::Letter kept)
	: Word(kept.id(), kept.word().saying())
	, place_(Place::of(kept.placement().place()))
	, author_(Soul::of(kept.word().author()))
	, created_at_(kept.dating().created_at())
{}


} // namespace will::domain
