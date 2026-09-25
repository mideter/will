#include "obedience.h"

#include "beings/testator.h"


namespace will::domain {


Obedience::Obedience(const Soul& testator)
	: testator_(static_cast<const Testator&>(testator))
{}


} // namespace will::domain
