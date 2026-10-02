#include "spirit.h"

#include "beings/immanents/abode.h"
#include "beings/immanents/soul.h"


namespace will::domain {


const Soul& Spirit::soul(const id::Soul id)
{
	return heaven().soul(id);
}


matter::Utterance Spirit::utter(const Saying& saying) const
{
	const auto& self = static_cast<const Soul&>(*this);
	return eternity().utter(self.id(), saying);
}


std::vector<matter::Utterance> Spirit::utterances(const std::vector<id::Word>& ids) const
{
	return eternity().utterances(ids);
}


bool Spirit::knows(const id::Soul soul_id) const
{
	return heaven().knows(soul_id);
}


void Spirit::contemplate(const Abode& abode) const
{
	const auto& self = static_cast<const Soul&>(*this);
	heaven().contemplate(self, abode);
}


const Contemplation& Spirit::contemplation() const
{
	const auto& self = static_cast<const Soul&>(*this);
	return heaven().contemplation(self.id());
}


} // namespace will::domain
