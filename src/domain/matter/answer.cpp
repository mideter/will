#include "answer.h"

#include <stdexcept>


namespace will::domain::matter {


Answer::Answer(const id::Soul suppliant, const id::Soul addressee, const Form form)
	: suppliant_(suppliant)
	, addressee_(addressee)
	, form_(form)
{
	if (suppliant_ == addressee_)
		throw std::invalid_argument("answer requires distinct suppliant and addressee");
}


} // namespace will::domain::matter
