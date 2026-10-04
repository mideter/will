#include "spirit.h"

#include "immanents/abode.h"
#include "immanents/soul.h"


namespace will::domain {


const Soul& Spirit::soul(const id::Soul id)
{
	return heaven().soul(id);
}


matter::Word Spirit::utter(const Saying& saying) const
{
	const auto& self = static_cast<const Soul&>(*this);
	return eternity().utter(self.id(), saying);
}


std::vector<matter::Word> Spirit::words(const std::vector<id::Word>& ids) const
{
	return eternity().words(ids);
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


void Spirit::cease() const
{
	const auto& self = static_cast<const Soul&>(*this);
	heaven().cease(self);
}


std::shared_ptr<const Contemplation> Spirit::contemplation() const
{
	const auto& self = static_cast<const Soul&>(*this);
	return heaven().contemplation(self.id());
}


} // namespace will::domain
