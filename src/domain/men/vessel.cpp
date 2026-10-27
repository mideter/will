#include "vessel.h"


namespace will::domain {


Vessel::Vessel(matter::Vessel kept)
	: id_(kept.id())
	, token_(kept.token())
{}


} // namespace will::domain
