#include "obedience.h"

#include "beings/soul.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Obedience::Obedience(const id::Obedience id, const Soul& testator, const Soul& executor)
	: Place(id::Place{id.value()})
	, testator_(testator)
	, executor_(executor)
{
	if (testator_.id() == executor_.id())
		throw std::invalid_argument("obedience requires distinct testator and executor");
}


Obedience::Obedience(Obedience&& other) noexcept
	: Place(std::move(other))
	, testator_(other.testator_)
	, executor_(other.executor_)
{}


} // namespace will::domain
