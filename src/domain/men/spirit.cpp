#include "spirit.h"

#include "places/abode.h"
#include "men/soul.h"

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


Timestamp Spirit::now() const
{
	return eternity().time().instant();
}


matter::Training Spirit::exercise(const id::Word word, std::vector<Exercise> exercises) const
{
	return eternity().train(word, std::move(exercises));
}


bool Spirit::knows(const id::Soul soul_id) const
{
	return heaven().knows(soul_id);
}


void Spirit::contemplate(const Place& place) const
{
	const auto& self = static_cast<const Soul&>(*this);
	heaven().contemplate(self, place);
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
