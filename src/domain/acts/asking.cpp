#include "asking.h"

#include <stdexcept>


namespace will::domain {


Asking::Asking(const id::Soul suppliant, const id::Soul addressee)
	: suppliant_(suppliant)
	, addressee_(addressee)
{
	if (suppliant_ == addressee_)
		throw std::invalid_argument("asking requires distinct suppliant and addressee");
}


} // namespace will::domain
