#include "shepherding.h"


namespace will::domain {


Shepherding::Shepherding(const id::Tie id, const Novice& novice)
	: Place(id::Place{id.value()})
	, novice_(novice)
{}


} // namespace will::domain
