#include "witness.h"

#include "matter/utterance.h"
#include "beings/immanents/abode.h"
#include "ports/spatiality.h"
#include "ports/temporality.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Witness::Witness(matter::Man kept)
	: Man(std::move(kept))
{
	contemplate(abode());
}


void Witness::contemplate(const Abode& abode) const
{
	Spirit::contemplate(abode);
}


void Witness::say(const Saying& saying) const
{
	const Abode& place = contemplation().abode();
	if (!place.dwells(*this))
		throw std::logic_error("Witness does not dwell in the contemplated abode");

	const matter::Utterance uttered = utter(saying);
	spatiality().place(uttered.id(), place.id());
	temporality().date(uttered.id());
}


} // namespace will::domain
