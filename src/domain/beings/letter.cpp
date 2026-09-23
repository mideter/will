#include "letter.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Letter::Letter(Inscription inscription, const Place& place)
	: Word(inscription.word())
	, id_(inscription.id())
	, place_(place)
	, author_(Soul::of(inscription.author()))
	, created_at_(inscription.created_at())
{
	if (place.id() != inscription.place())
		throw std::invalid_argument("letter place must match inscription");
}


} // namespace will::domain
