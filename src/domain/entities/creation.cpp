#include "creation.h"


namespace will::domain {


Creation::Creation(Temporality& temporality)
	: world_(Heaven{temporality}, Earth{temporality})
{
	world_.awaken();
}


} // namespace will::domain
