#include "tie.h"

#include <stdexcept>


namespace will::domain::matter {


Tie::Tie(const id::Place id, const id::Soul testator, const id::Soul novice)
	: id_(id)
	, testator_(testator)
	, novice_(novice)
{
	if (testator_ == novice_)
		throw std::invalid_argument("tie requires distinct testator and novice");
}


} // namespace will::domain::matter
