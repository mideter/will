#include "saying.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Saying::Saying(std::string body)
	: body_(std::move(body))
{
	if (body_.empty())
		throw std::invalid_argument("Saying must be non-empty");
	if (body_.size() > MaxBodyLength)
		throw std::invalid_argument("Saying exceeds MaxBodyLength");
}


} // namespace will::domain
