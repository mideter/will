#include "spirit.h"

#include "beings/immanents/abode.h"
#include "beings/immanents/soul.h"


namespace will::domain {


Utterance Spirit::utter(const Saying& saying) const
{
	const auto& soul = static_cast<const Soul&>(*this);
	return eternity().utter(soul.id(), saying);
}


std::vector<Utterance> Spirit::utterances(const std::vector<id::Letter>& ids) const
{
	return eternity().utterances(ids);
}


bool Spirit::knows(const id::Soul soul_id) const
{
	return heaven().knows(soul_id);
}


void Spirit::contemplate(const Abode& abode) const
{
	const auto& soul = static_cast<const Soul&>(*this);
	heaven().contemplate(soul, abode);
}


const Contemplation& Spirit::contemplation() const
{
	const auto& soul = static_cast<const Soul&>(*this);
	return heaven().contemplation(soul.id());
}


} // namespace will::domain
