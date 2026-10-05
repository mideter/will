#include "contemplation.h"

#include "men/witness.h"
#include "words/recollection.h"


namespace will::domain {


Contemplation::Contemplation(Birth<Heaven>, const Witness& who, const Place& place)
	: who_(who)
	, place_(place)
	, recollection_(place.recollection(*this))
{}


std::vector<std::shared_ptr<const Word>> Contemplation::words() const
{
	std::vector<std::shared_ptr<const Word>> shown;
	for (std::shared_ptr<const Word>& word : recollection_->words()) {
		if (place_.shows(who_, *word))
			shown.push_back(std::move(word));
	}

	return shown;
}


} // namespace will::domain
