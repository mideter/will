#include "contemplation.h"

#include "immanents/witness.h"
#include "words/recollection.h"


namespace will::domain {


Contemplation::Contemplation(Birth<Heaven>, const Witness& who, const Place& place)
	: who_(who)
	, place_(place)
	, recollection_(place.recollection(*this))
{}


std::vector<std::shared_ptr<const Word>> Contemplation::words() const
{
	return recollection_->words();
}


} // namespace will::domain
