#include "witness.h"

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


void Witness::contemplate(const Abode& abode) const
{
	Spirit::contemplate(abode);
}


void Witness::say(const Saying& saying) const
{
	const std::shared_ptr<const Contemplation> gaze = contemplation();
	if (!gaze)
		throw std::logic_error("Witness is asleep");

	const Abode& place = gaze->abode();
	if (!place.dwells(*this))
		throw std::logic_error("Witness does not dwell in the contemplated abode");

	const matter::Word uttered = utter(saying);
	spatiality().place(uttered.id(), place.id());
	temporality().date(uttered.id());
}


} // namespace will::domain
