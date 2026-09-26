#include "tie.h"

#include "beings/soul.h"
#include "beings/space.h"
#include "beings/testator.h"
#include "properties/immanent.h"


namespace will::domain {


Tie::Tie(Tying tying)
	: Place(id::Place{tying.id().value()})
	, Obedience(Soul::of(tying.testator()))
	, Shepherding(Soul::of(tying.novice()))
{
	Immanent<Space>::present<Place>();
}


Tie::~Tie()
{
	testator_.drop(*this);
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
