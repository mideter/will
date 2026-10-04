#include "execution.h"

#include <stdexcept>


namespace will::domain::matter {


Execution::Execution(const id::Word deed, const id::Word behest)
	: deed_(deed)
	, behest_(behest)
{
	if (deed_ == behest_)
		throw std::invalid_argument("a deed fulfils another word than itself");
}


} // namespace will::domain::matter
