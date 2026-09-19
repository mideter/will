#include "shepherding.h"

#include "beings/novice.h"
#include "beings/soul.h"
#include "beings/testator.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Shepherding::Shepherding(const id::Obedience id, const Testator& testator, const Novice& novice)
	: Place(id::Place{id.value()})
	, testator_(testator)
	, novice_(novice)
{
	if (testator_.Soul::id() == novice_.Soul::id())
		throw std::invalid_argument("shepherding requires distinct testator and novice");
}


Shepherding::Shepherding(Shepherding&& other) noexcept
	: Place(std::move(other))
	, testator_(other.testator_)
	, novice_(other.novice_)
{}


} // namespace will::domain
