#include "remark.h"

#include <stdexcept>
#include <utility>


namespace will::domain::matter {


Remarked::Remarked(const id::Word remark, const id::Word training, const std::uint32_t exercise)
	: remark_(remark)
	, training_(training)
	, exercise_(exercise)
{
	if (remark_ == training_)
		throw std::invalid_argument("a remark is said on another word than itself");
}


Remark::Remark(Word word, Remarked on, Dating dating)
	: word_(std::move(word))
	, on_(on)
	, dating_(std::move(dating))
{
	if (word_.id() != on_.id() || word_.id() != dating_.id())
		throw std::invalid_argument("remark parts must share the word id");
}


} // namespace will::domain::matter
