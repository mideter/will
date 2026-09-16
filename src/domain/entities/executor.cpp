#include "executor.h"


namespace will::domain {


Executor::Executor(Man&& man, Abode& abode) noexcept
	: Witness(std::move(man), abode)
{}


} // namespace will::domain
