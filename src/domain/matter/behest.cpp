#include "behest.h"

#include <stdexcept>
#include <utility>


namespace will::domain::matter {


Behest::Behest(Word word, Placement placement, Dating dating, std::optional<Training> training)
	: word_(std::move(word))
	, placement_(std::move(placement))
	, dating_(std::move(dating))
	, training_(std::move(training))
{
	if (word_.id() != placement_.id() || word_.id() != dating_.id())
		throw std::invalid_argument("behest parts must share the word id");
	if (training_ && training_->id() != word_.id())
		throw std::invalid_argument("behest parts must share the word id");
}


} // namespace will::domain::matter
