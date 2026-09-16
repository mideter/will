#include "testator.h"

#include <utility>


namespace will::domain {


Testator::Testator(Man&& man)
	: Executor(std::move(man))
{}


} // namespace will::domain
