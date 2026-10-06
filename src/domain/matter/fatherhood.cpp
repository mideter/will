#include "fatherhood.h"

#include <stdexcept>


namespace will::domain::matter {


Fatherhood::Fatherhood(const id::Soul father, const id::Soul child, const Line line)
	: father_(father)
	, child_(child)
	, line_(line)
{
	if (father_ == child_)
		throw std::invalid_argument("fatherhood requires distinct father and child");
}


} // namespace will::domain::matter
