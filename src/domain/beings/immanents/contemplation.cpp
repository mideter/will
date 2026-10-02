#include "contemplation.h"

#include "beings/immanents/witness.h"
#include "beings/letter.h"


namespace will::domain {


Contemplation::Contemplation(const Witness& who, const Abode& abode)
	: who_(who)
	, abode_(abode)
{}


std::vector<Letter> Contemplation::letters() const
{
	return abode_.letters(*this);
}


} // namespace will::domain
