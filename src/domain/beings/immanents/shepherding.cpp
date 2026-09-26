#include "shepherding.h"

#include "beings/immanents/novice.h"


namespace will::domain {


Shepherding::Shepherding(const Soul& novice)
	: novice_(static_cast<const Novice&>(novice))
{}


} // namespace will::domain
