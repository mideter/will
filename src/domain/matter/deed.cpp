#include "deed.h"

#include <stdexcept>
#include <utility>


namespace will::domain::matter {


Deed::Deed(Word word, Placement placement, Dating dating, Execution execution)
	: word_(std::move(word))
	, placement_(std::move(placement))
	, dating_(std::move(dating))
	, execution_(std::move(execution))
{
	if (word_.id() != placement_.id() || word_.id() != dating_.id() || word_.id() != execution_.id())
		throw std::invalid_argument("deed parts must share the word id");
}


} // namespace will::domain::matter
