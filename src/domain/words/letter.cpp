#include "letter.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Letter::Letter(matter::Letter kept)
	: Word(kept.id())
	, place_(Place::of(kept.placement().place()))
	, author_(Soul::of(kept.word().author()))
	, saying_(kept.word().saying())
	, created_at_(kept.dating().created_at())
{}


} // namespace will::domain
