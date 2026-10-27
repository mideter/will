#include "soul.h"


namespace will::domain {


Soul::Soul(matter::Soul kept)
	: id_(kept.id())
	, name_(kept.name())
{}


const Soul& Soul::of(const id::Soul id)
{
	return horizon().soul(id);
}


} // namespace will::domain
