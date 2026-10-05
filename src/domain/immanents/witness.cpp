#include "witness.h"

#include "matter/dating.h"
#include "matter/letter.h"
#include "matter/placement.h"
#include "words/letter.h"
#include "matter/word.h"
#include "immanents/abode.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"

#include <memory>
#include <stdexcept>
#include <utility>


namespace will::domain {


Witness::Witness(matter::Man kept)
	: Man(std::move(kept))
{}


void Witness::wake() const
{
	contemplate(abode());
}


void Witness::sleep() const
{
	cease();
}


void Witness::contemplate(const Place& place) const
{
	if (!place.dwells(*this))
		throw std::logic_error("Witness does not dwell in this place");

	Spirit::contemplate(place);
}


bool Witness::contemplates(const Place& place) const
{
	const std::shared_ptr<const Contemplation> gaze = contemplation();
	return gaze && &gaze->place() == &place;
}


void Witness::say(const Saying& saying) const
{
	const std::shared_ptr<const Contemplation> gaze = contemplation();
	if (!gaze)
		throw std::logic_error("Witness is asleep");

	const auto* abode = dynamic_cast<const Abode*>(&gaze->place());
	if (!abode)
		throw std::logic_error("one says only in an abode");

	const Abode& place = *abode;

	matter::Word uttered = utter(saying);
	matter::Placement placed = spatiality().place(uttered.id(), place.id());
	matter::Dating dated = temporality().date(uttered.id());

	const std::shared_ptr<const Letter> letter =
		place.inscribe(*gaze, matter::Letter{std::move(uttered), std::move(placed), std::move(dated)});
	for (const std::shared_ptr<const Contemplation>& beholder : gazes(place))
		beholder->behold(letter);
}


} // namespace will::domain
