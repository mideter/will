#include "contemplation.h"

#include "immanents/witness.h"
#include "words/letter.h"


namespace will::domain {


Contemplation::Contemplation(const Witness& who, const Abode& abode)
	: who_(who)
	, abode_(abode)
{}


std::vector<std::shared_ptr<const Letter>> Contemplation::letters() const
{
	return abode_.letters(*this);
}


} // namespace will::domain
