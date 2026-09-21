#include "deed.h"

#include "beings/novice.h"
#include "beings/soul.h"
#include "beings/testator.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Deed::Deed(const id::Deed id, const Obedience& obedience, Word word, const Timestamp created_at,
		   std::optional<Timestamp> executed_at, std::optional<Timestamp> cancelled_at)
	: Deed(id, id::Tie{obedience.id().value()}, obedience.testator(), obedience.novice(),
		   std::move(word), created_at, std::move(executed_at), std::move(cancelled_at))
{}


Deed::Deed(const id::Deed id, const id::Tie place, const Testator& testator,
		   const Novice& novice, Word word, const Timestamp created_at,
		   std::optional<Timestamp> executed_at, std::optional<Timestamp> cancelled_at)
	: Letter(id::Letter{id.value()}, id::Place{place.value()}, testator.Soul::id(), std::move(word),
			 created_at)
	, tie_id_(place)
	, testator_(testator)
	, novice_(novice)
	, executed_at_(std::move(executed_at))
	, cancelled_at_(std::move(cancelled_at))
{
	if (executed_at_ && cancelled_at_)
		throw std::invalid_argument("deed cannot be both executed and cancelled");
}


} // namespace will::domain
