#include "tie.h"

#include "beings/immanents/soul.h"
#include "beings/space.h"
#include "beings/immanents/testator.h"
#include "properties/immanent.h"


namespace will::domain {


Tie::Tie(Boundness boundness)
	: Place(id::Place{boundness.id().value()})
	, Obedience(Soul::of(boundness.testator()))
	, Shepherding(Soul::of(boundness.novice()))
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
