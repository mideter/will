#include "execution.h"


namespace will::domain {


Execution::Execution(const id::Word id, const Timestamp executed_at)
	: id_(id)
	, executed_at_(executed_at)
{}


} // namespace will::domain
