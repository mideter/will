#include "behest.h"

#include <stdexcept>
#include <utility>


namespace will::domain::matter {


Behest::Behest(Word word, Placement placement, Dating dating, std::optional<Training> training,
			   std::vector<Effort> efforts, std::optional<Underway> underway)
	: word_(std::move(word))
	, placement_(std::move(placement))
	, dating_(std::move(dating))
	, training_(std::move(training))
	, efforts_(std::move(efforts))
	, underway_(std::move(underway))
{
	if (word_.id() != placement_.id() || word_.id() != dating_.id())
		throw std::invalid_argument("behest parts must share the word id");
	if (training_ && training_->id() != word_.id())
		throw std::invalid_argument("behest parts must share the word id");
	for (const Effort& effort : efforts_) {
		if (!training_ || effort.training() != word_.id())
			throw std::invalid_argument("efforts are of the training of this behest");
	}
	if (underway_ && (!training_ || underway_->training() != word_.id()))
		throw std::invalid_argument("the approach underway is of the training of this behest");
}


} // namespace will::domain::matter
