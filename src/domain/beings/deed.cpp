#include "deed.h"

#include "beings/soul.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Deed::Deed(const id::Deed id, const Obedience& obedience, Word word, const Timestamp created_at,
		   std::optional<Timestamp> executed_at, std::optional<Timestamp> cancelled_at)
	: Letter(id::Letter{id.value()}, obedience.id(), obedience.testator().id(), std::move(word),
			 created_at)
	, obedience_id_(obedience.obedience_id())
	, testator_(obedience.testator())
	, executor_(obedience.executor())
	, executed_at_(std::move(executed_at))
	, cancelled_at_(std::move(cancelled_at))
{
	if (executed_at_ && cancelled_at_)
		throw std::invalid_argument("deed cannot be both executed and cancelled");
}


} // namespace will::domain
