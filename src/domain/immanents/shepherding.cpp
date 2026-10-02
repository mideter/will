#include "shepherding.h"

#include "immanents/novice.h"


namespace will::domain {


Shepherding::Shepherding(const Soul& novice)
	: novice_(static_cast<const Novice&>(novice))
{}


} // namespace will::domain
