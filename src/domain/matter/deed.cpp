#include "deed.h"

#include <stdexcept>
#include <utility>


namespace will::domain::matter {


Deed::Deed(Utterance utterance, Placement placement, Dating dating, std::optional<Execution> execution)
	: utterance_(std::move(utterance))
	, placement_(std::move(placement))
	, dating_(std::move(dating))
	, execution_(std::move(execution))
{
	if (utterance_.id() != placement_.id() || utterance_.id() != dating_.id())
		throw std::invalid_argument("deed parts must share the word id");
	if (execution_ && execution_->id() != utterance_.id())
		throw std::invalid_argument("deed parts must share the word id");
}


} // namespace will::domain::matter
