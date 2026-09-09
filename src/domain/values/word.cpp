#include "word.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Word::Word(std::string body)
	: body_(std::move(body))
{
	if (body_.empty())
		throw std::invalid_argument("Word must be non-empty");
	if (body_.size() > MaxBodyLength)
		throw std::invalid_argument("Word exceeds MaxBodyLength");
}


} // namespace will::domain
