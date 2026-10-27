#include "abode.h"


namespace will::domain::matter {


Abode::Abode(const id::Place id, const id::Place gates, const id::Place helm)
	: id_(id)
	, gates_(gates)
	, helm_(helm)
{}


} // namespace will::domain::matter
