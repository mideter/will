#include "room.h"


namespace will::domain::matter {


Room::Room(const id::Place id, const id::Place abode, const id::Place reflects, const Aspect aspect, const Part part)
	: id_(id)
	, abode_(abode)
	, reflects_(reflects)
	, aspect_(aspect)
	, part_(part)
{}


} // namespace will::domain::matter
