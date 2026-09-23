#include "letter.h"


namespace will::domain {


Letter::Letter(Inscription inscription)
	: Word(inscription.word())
	, id_(inscription.id())
	, place_(Place::of(inscription.place()))
	, author_(Soul::of(inscription.author()))
	, created_at_(inscription.created_at())
{}


} // namespace will::domain
