#include "tie.h"

#include "beings/novice.h"
#include "beings/soul.h"
#include "beings/testator.h"


namespace will::domain {


Tie::Tie(Tying tying)
	: Place(id::Place{tying.id().value()})
	, Obedience(static_cast<const Testator&>(Soul::of(tying.testator())))
	, Shepherding(static_cast<const Novice&>(Soul::of(tying.novice())))
{
	present();
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
