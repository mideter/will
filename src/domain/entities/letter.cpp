#include "letter.h"

#include "entities/heaven.h"
#include "entities/soul.h"

#include <utility>


namespace will::domain {


Letter::Letter(const id::Letter id, const id::Place place_id, const id::Soul author_id, Word word,
			   const Timestamp created_at)
	: Word(std::move(word))
	, id_(id)
	, place_id_(place_id)
	, author_id_(author_id)
	, created_at_(created_at)
{}


const Soul& Letter::author() const
{
	return Heaven::the().soul(author_id_);
}


} // namespace will::domain
