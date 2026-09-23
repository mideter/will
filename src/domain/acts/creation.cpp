#include "creation.h"

#include "beings/space.h"


namespace will::domain {


Creation::Creation(Temporality& temporality)
	: world_(Heaven{temporality}, Earth{temporality}, Space{})
{
	world_.awaken();
}


} // namespace will::domain
