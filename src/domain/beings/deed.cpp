#include "deed.h"

#include "beings/soul.h"
#include "beings/testator.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Deed::Deed(const id::Deed id, const Tie& tie, Word word, const Timestamp created_at,
		   std::optional<Timestamp> executed_at, std::optional<Timestamp> cancelled_at)
	: Letter(id::Letter{id.value()}, tie.id(), tie.testator().Soul::id(), std::move(word),
			 created_at)
	, tie_(tie)
	, executed_at_(std::move(executed_at))
	, cancelled_at_(std::move(cancelled_at))
{
	if (executed_at_ && cancelled_at_)
		throw std::invalid_argument("deed cannot be both executed and cancelled");
}


} // namespace will::domain
