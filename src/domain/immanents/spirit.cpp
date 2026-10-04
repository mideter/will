#include "spirit.h"

#include "immanents/abode.h"
#include "immanents/soul.h"

#include <utility>


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


std::vector<std::shared_ptr<const Contemplation>> Spirit::gazes(const Abode& abode)
{
	return heaven().gazes(abode);
}


std::vector<std::shared_ptr<const Supplication>> Spirit::supplications() const
{
	const auto& self = static_cast<const Soul&>(*this);
	return heaven().supplications(self.id());
}


void Spirit::keep(std::shared_ptr<const Supplication> supplication)
{
	heaven().keep(std::move(supplication));
}


void Spirit::release(const Supplication& supplication)
{
	heaven().release(supplication);
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
