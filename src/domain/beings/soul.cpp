#include "soul.h"


namespace will::domain {


Soul::Soul(const id::Soul id, SoulName name)
	: id_(id)
	, name_(std::move(name))
{}


const Soul& Soul::of(const id::Soul id)
{
	return the_heaven().soul(id);
}


} // namespace will::domain
