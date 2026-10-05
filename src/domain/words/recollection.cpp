#include "recollection.h"

#include "immanents/place.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Recollection::Recollection(Birth<Life>, const Place& place, std::vector<std::shared_ptr<const Word>> words)
	: place_(place)
	, words_(std::move(words))
{}


std::vector<std::shared_ptr<const Word>> Recollection::words() const
{
	std::lock_guard lock(mutex_);
	return words_;
}


void Recollection::enter(const Birth<Place> birth, std::shared_ptr<const Word> word) const
{
	if (&birth.parent() != &place_)
		throw std::logic_error("a word enters only the recollection of its place");

	std::lock_guard lock(mutex_);
	words_.push_back(std::move(word));
}


} // namespace will::domain
