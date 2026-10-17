#include "contemplation.h"

#include "men/man.h"
#include "places/room.h"
#include "refusals/place.h"
#include "words/recollection.h"


namespace will::domain {


Contemplation::Contemplation(Birth<Heaven>, const Man& who, const Place& place)
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


const Cell& Contemplation::cell() const
{
	const auto* cell = dynamic_cast<const Cell*>(&place_);
	if (!cell)
		throw StandInCell();
	return *cell;
}


const UpperRoom& Contemplation::upper_room() const
{
	const auto* upper_room = dynamic_cast<const UpperRoom*>(&place_);
	if (!upper_room)
		throw StandInUpperRoom();
	return *upper_room;
}


const Gates& Contemplation::gates() const
{
	const auto* gates = dynamic_cast<const Gates*>(&place_);
	if (!gates)
		throw StandAtGates();
	return *gates;
}


bool Contemplation::writable() const
{
	const auto* cell = dynamic_cast<const Cell*>(&place_);
	return cell && cell->writable();
}


} // namespace will::domain
