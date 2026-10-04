#include "behest.h"

#include <stdexcept>
#include <utility>


namespace will::domain::matter {


Behest::Behest(Word word, Placement placement, Dating dating, std::optional<Execution> execution)
	: word_(std::move(word))
	, placement_(std::move(placement))
	, dating_(std::move(dating))
	, execution_(std::move(execution))
{
	if (word_.id() != placement_.id() || word_.id() != dating_.id())
		throw std::invalid_argument("behest parts must share the word id");
	if (execution_ && execution_->id() != word_.id())
		throw std::invalid_argument("behest parts must share the word id");
}


} // namespace will::domain::matter
