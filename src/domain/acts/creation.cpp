#include "creation.h"


namespace will::domain {


Creation::Creation(Eternity& eternity, Spatiality& spatiality, Temporality& temporality)
	: world_(eternity, temporality, spatiality)
{
	world_.awaken();
}


} // namespace will::domain
