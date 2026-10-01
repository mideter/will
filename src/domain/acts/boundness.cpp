#include "boundness.h"

#include <stdexcept>


namespace will::domain {


Boundness::Boundness(const id::Tie id, const id::Soul testator, const id::Soul novice)
	: id_(id)
	, testator_(testator)
	, novice_(novice)
{
	if (testator_ == novice_)
		throw std::invalid_argument("boundness requires distinct testator and novice");
}


} // namespace will::domain
