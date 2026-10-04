#include "contemplation.h"

#include "immanents/witness.h"
#include "words/behest.h"
#include "words/deed.h"
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
	const Place* where = nullptr;
	if (const auto* letter = dynamic_cast<const Letter*>(word.get()))
		where = &letter->place();
	else if (const auto* behest = dynamic_cast<const Behest*>(word.get()))
		where = &behest->tie();
	else if (const auto* deed = dynamic_cast<const Deed*>(word.get()))
		where = &deed->tie();

	if (where != &place_)
		throw std::logic_error("the word is not placed in the contemplated place");

	std::lock_guard lock(mutex_);
	words_.push_back(std::move(word));
}


} // namespace will::domain
