#include "obedience.h"

#include "beings/executor.h"
#include "beings/soul.h"
#include "beings/testator.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Obedience::Obedience(const id::Obedience id, const Testator& testator, const Executor& executor)
	: Place(id::Place{id.value()})
	, testator_(testator)
	, executor_(executor)
{
	if (testator_.Soul::id() == executor_.Soul::id())
		throw std::invalid_argument("obedience requires distinct testator and executor");
}


Obedience::Obedience(Obedience&& other) noexcept
	: Place(std::move(other))
	, testator_(other.testator_)
	, executor_(other.executor_)
{}


} // namespace will::domain
