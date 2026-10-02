#include "letter.h"

#include <stdexcept>
#include <utility>


namespace will::domain::matter {


Letter::Letter(Utterance utterance, Placement placement, Dating dating)
	: utterance_(std::move(utterance))
	, placement_(std::move(placement))
	, dating_(std::move(dating))
{
	if (utterance_.id() != placement_.id() || utterance_.id() != dating_.id())
		throw std::invalid_argument("letter parts must share the word id");
}


} // namespace will::domain::matter
