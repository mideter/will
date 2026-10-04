#include "life.h"

#include "acts/creation.h"
#include "dimensions/eternity.h"

#include <stdexcept>


namespace will::domain {


Life* Life::current_ = nullptr;


Life::Life()
{
	(void)Eternity::the();

	if (current_ != nullptr)
		throw std::logic_error("Only one Life");

	current_ = this;
}


Life::~Life()
{
	if (current_ == this)
		current_ = nullptr;
}


Creation Life::create(Spatiality& spatiality, Temporality& temporality)
{
	return Creation{Eternity::the(), spatiality, temporality};
}


} // namespace will::domain
