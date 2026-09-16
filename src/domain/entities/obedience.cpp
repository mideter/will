#include "obedience.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Obedience::Obedience(const id::Obedience id, const id::Soul testator, const id::Soul executor,
					 const bool living)
	: Place(id::Place{id.value()})
	, testator_(testator)
	, executor_(executor)
	, living_(living)
{
	if (testator_ == executor_)
		throw std::invalid_argument("obedience requires distinct testator and executor");
}


Obedience::Obedience(Obedience&& other) noexcept
	: Place(std::move(other))
	, testator_(other.testator_)
	, executor_(other.executor_)
	, living_(other.living_)
{}


Obedience& Obedience::operator=(Obedience&& other) noexcept
{
	if (this == &other)
		return *this;
	Place::operator=(std::move(other));
	testator_ = other.testator_;
	executor_ = other.executor_;
	living_ = other.living_;
	return *this;
}


} // namespace will::domain
