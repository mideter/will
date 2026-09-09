#include "letter.h"

#include <utility>


namespace will::domain {


Letter::Letter(const id::Letter id, const id::Abode abode_id, const id::Soul author_id, Word word,
			   const Timestamp created_at)
	: Word(std::move(word))
	, id_(id)
	, abode_id_(abode_id)
	, author_id_(author_id)
	, created_at_(created_at)
{}


} // namespace will::domain
