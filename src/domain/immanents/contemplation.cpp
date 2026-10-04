#include "contemplation.h"

#include "immanents/witness.h"
#include "words/letter.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Contemplation::Contemplation(Birth<Heaven>, const Witness& who, const Place& place)
	: who_(who)
	, place_(place)
	, words_(place.words(*this))
{}


std::vector<std::shared_ptr<const Word>> Contemplation::words() const
{
	std::lock_guard lock(mutex_);
	return words_;
}


void Contemplation::behold(std::shared_ptr<const Word> word) const
{
	const auto* letter = dynamic_cast<const Letter*>(word.get());
	if (!letter || &letter->place() != &place_)
		throw std::logic_error("the word is not placed in the contemplated place");

	std::lock_guard lock(mutex_);
	words_.push_back(std::move(word));
}


} // namespace will::domain
