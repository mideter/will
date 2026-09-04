#include "letter.h"

#include <stdexcept>


namespace will::domain {


Letter::Letter(const id::Letter id, const id::Abode abode_id, const id::Soul author_id, std::string body,
			   const Timestamp created_at)
	: id_(id)
	, abode_id_(abode_id)
	, author_id_(author_id)
	, body_(std::move(body))
	, created_at_(created_at)
{
	if (body_.empty())
		throw std::invalid_argument("Letter body must be non-empty");
	if (body_.size() > MaxBodyLength)
		throw std::invalid_argument("Letter body exceeds MaxBodyLength");
}


} // namespace will::domain
