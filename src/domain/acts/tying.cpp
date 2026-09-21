#include "tying.h"

#include <stdexcept>


namespace will::domain {


Tying::Tying(const id::Obedience id, const id::Soul testator, const id::Soul novice)
	: id_(id)
	, testator_(testator)
	, novice_(novice)
{
	if (testator_ == novice_)
		throw std::invalid_argument("tying requires distinct testator and novice");
}


} // namespace will::domain
