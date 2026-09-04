#include "man.h"


namespace will::domain {


Man::Man(const id::Man id, const id::Soul soul_id, const id::Vessel vessel_id)
	: id_(id)
	, soul_id_(soul_id)
	, vessel_id_(vessel_id)
{}


} // namespace will::domain
