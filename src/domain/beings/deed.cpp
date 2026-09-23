#include "deed.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Deed::Deed(const id::Deed id, const Tie& tie, Word word, const Timestamp created_at,
		   std::optional<Timestamp> executed_at, std::optional<Timestamp> cancelled_at)
	: Word(std::move(word))
	, id_(id)
	, tie_(tie)
	, created_at_(created_at)
	, executed_at_(std::move(executed_at))
	, cancelled_at_(std::move(cancelled_at))
{
	if (executed_at_ && cancelled_at_)
		throw std::invalid_argument("deed cannot be both executed and cancelled");
}


} // namespace will::domain
