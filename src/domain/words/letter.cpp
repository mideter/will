#include "letter.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Letter::Letter(matter::Letter kept)
	: id_(kept.id())
	, place_(Place::of(kept.placement().place()))
	, author_(Soul::of(kept.word().author()))
	, saying_(kept.word().saying())
	, created_at_(kept.dating().created_at())
{}


} // namespace will::domain
