#include "testator.h"


namespace will::domain {


Testator::Testator(Man&& man, Abode& abode) noexcept
	: Executor(std::move(man), abode)
{}


} // namespace will::domain
