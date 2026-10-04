#include "contemplation.h"

#include "immanents/witness.h"
#include "words/letter.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Contemplation::Contemplation(const Witness& who, const Abode& abode)
	: who_(who)
	, abode_(abode)
	, letters_(abode.letters(*this))
{}


std::vector<std::shared_ptr<const Letter>> Contemplation::letters() const
{
	std::lock_guard lock(mutex_);
	return letters_;
}


void Contemplation::behold(std::shared_ptr<const Letter> letter) const
{
	if (&letter->place() != &abode_)
		throw std::logic_error("the letter is not said in the contemplated abode");

	std::lock_guard lock(mutex_);
	letters_.push_back(std::move(letter));
}


} // namespace will::domain
