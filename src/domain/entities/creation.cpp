#include "creation.h"

#include "entities/earth.h"
#include "entities/heaven.h"


namespace will::domain {


Creation::Creation(Temporality& temporality)
	: world_(Heaven{temporality}, Earth{temporality})
{}


Creation::~Creation()
{
	static_cast<Earth&>(world_).lower();
	static_cast<Heaven&>(world_).lower();
}


} // namespace will::domain
