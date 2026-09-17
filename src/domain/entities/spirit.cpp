#include "spirit.h"

#include "entities/soul.h"
#include "identity/soul.h"

#include <stdexcept>


namespace will::domain {


const Soul& Spirit::known(const id::Soul id)
{
	return Heaven::the().soul(id);
}


bool Spirit::knows(const id::Soul id)
{
	return Heaven::the().knows(id);
}


void Spirit::present(const Soul& soul) const
{
	heaven().present(soul);
}


void Spirit::say(const Word&) const
{
	throw std::logic_error("Spirit outside the living world cannot say");
}


} // namespace will::domain
