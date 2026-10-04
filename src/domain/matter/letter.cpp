#include "letter.h"

#include <stdexcept>
#include <utility>


namespace will::domain::matter {


Letter::Letter(Word word, Placement placement, Dating dating)
	: word_(std::move(word))
	, placement_(std::move(placement))
	, dating_(std::move(dating))
{
	if (word_.id() != placement_.id() || word_.id() != dating_.id())
		throw std::invalid_argument("letter parts must share the word id");
}


} // namespace will::domain::matter
