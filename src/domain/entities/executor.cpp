#include "executor.h"

#include <utility>


namespace will::domain {


Executor::Executor(Man&& man)
	: Witness(std::move(man))
{}


} // namespace will::domain
