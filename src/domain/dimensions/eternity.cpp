#include "eternity.h"

#include <stdexcept>


namespace will::domain {


Eternity* Eternity::current_ = nullptr;


Eternity& Eternity::the()
{
	if (current_ == nullptr)
		throw std::logic_error("Eternity is not realised");

	return *current_;
}


Eternity::Eternity()
{
	if (current_ != nullptr)
		throw std::logic_error("Only one Eternity");

	current_ = this;
}


Eternity::~Eternity()
{
	if (current_ == this)
		current_ = nullptr;
}


} // namespace will::domain
