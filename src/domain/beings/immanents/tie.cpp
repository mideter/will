#include "tie.h"

#include "beings/immanents/soul.h"
#include "beings/space.h"
#include "beings/immanents/testator.h"
#include "properties/immanent.h"


namespace will::domain {


Tie::Tie(matter::Tie kept)
	: Place(id::Place{kept.id().value()})
	, Obedience(Soul::of(kept.testator()))
	, Shepherding(Soul::of(kept.novice()))
{
	Immanent<Space>::present<Place>();
}


Tie::~Tie()
{
	testator_.release(*this);
}


const Testator& Tie::testator() const
{
	return testator_;
}


const Novice& Tie::novice() const
{
	return novice_;
}


} // namespace will::domain
