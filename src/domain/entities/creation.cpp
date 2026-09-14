#include "creation.h"


namespace will::domain {


Creation::Creation(Temporality& temporality)
	: heaven_(temporality)
	, earth_(temporality)
	, world_()
{}


} // namespace will::domain
