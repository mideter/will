#include "supplication.h"

#include <stdexcept>


namespace will::domain::matter {


Supplication::Supplication(const id::Soul suppliant, const id::Soul addressee)
	: suppliant_(suppliant)
	, addressee_(addressee)
{
	if (suppliant_ == addressee_)
		throw std::invalid_argument("supplication requires distinct suppliant and addressee");
}


} // namespace will::domain::matter
