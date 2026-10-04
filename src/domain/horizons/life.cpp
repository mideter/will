#include "life.h"

#include "acts/creation.h"

#include <stdexcept>


namespace will::domain {


Life* Life::current_ = nullptr;


Life::Life()
{
	if (current_ != nullptr)
		throw std::logic_error("Only one Life");

	current_ = this;
}


Life::~Life()
{
	if (current_ == this)
		current_ = nullptr;
}


Creation Life::create(Eternity& eternity, Spatiality& spatiality, Temporality& temporality)
{
	return Creation{eternity, spatiality, temporality};
}


} // namespace will::domain
