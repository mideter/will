#include "inscription.h"

#include <utility>


namespace will::domain {


Inscription::Inscription(const id::Letter id, const id::Place place, const id::Soul author, Word word,
						 const Timestamp created_at)
	: id_(id)
	, place_(place)
	, author_(author)
	, word_(std::move(word))
	, created_at_(created_at)
{}


} // namespace will::domain
