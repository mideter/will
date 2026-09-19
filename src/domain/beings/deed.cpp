#include "deed.h"

#include "beings/novice.h"
#include "beings/soul.h"
#include "beings/testator.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Deed::Deed(const id::Deed id, const Obedience& obedience, Word word, const Timestamp created_at,
		   std::optional<Timestamp> executed_at, std::optional<Timestamp> cancelled_at)
	: Letter(id::Letter{id.value()}, obedience.id(), obedience.testator().Soul::id(), std::move(word),
			 created_at)
	, obedience_id_(obedience.obedience_id())
	, testator_(obedience.testator())
	, novice_(obedience.novice())
	, executed_at_(std::move(executed_at))
	, cancelled_at_(std::move(cancelled_at))
{
	if (executed_at_ && cancelled_at_)
		throw std::invalid_argument("deed cannot be both executed and cancelled");
}


Deed::Deed(const id::Deed id, const Shepherding& shepherding, Word word, const Timestamp created_at,
		   std::optional<Timestamp> executed_at, std::optional<Timestamp> cancelled_at)
	: Letter(id::Letter{id.value()}, shepherding.id(), shepherding.testator().Soul::id(),
			 std::move(word), created_at)
	, obedience_id_(shepherding.obedience_id())
	, testator_(shepherding.testator())
	, novice_(shepherding.novice())
	, executed_at_(std::move(executed_at))
	, cancelled_at_(std::move(cancelled_at))
{
	if (executed_at_ && cancelled_at_)
		throw std::invalid_argument("deed cannot be both executed and cancelled");
}


} // namespace will::domain
