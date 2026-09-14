#include "creation.h"

#include "entities/earth.h"
#include "entities/heaven.h"


namespace will::domain {


Creation::Creation(Temporality& temporality)
	: world_(Heaven{temporality}, Earth{temporality})
{}


} // namespace will::domain
