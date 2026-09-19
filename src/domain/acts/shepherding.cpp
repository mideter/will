#include "shepherding.h"

#include "beings/executor.h"
#include "beings/soul.h"
#include "beings/testator.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Shepherding::Shepherding(const id::Obedience id, const Testator& testator, const Executor& executor)
	: Place(id::Place{id.value()})
	, testator_(testator)
	, executor_(executor)
{
	if (testator_.Soul::id() == executor_.Soul::id())
		throw std::invalid_argument("shepherding requires distinct testator and executor");
}


Shepherding::Shepherding(Shepherding&& other) noexcept
	: Place(std::move(other))
	, testator_(other.testator_)
	, executor_(other.executor_)
{}


} // namespace will::domain
