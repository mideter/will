#include "spirit.h"

#include "men/soul.h"

#include <utility>


namespace will::domain {


matter::Word Spirit::utter(const Saying& saying) const
{
	const auto& self = static_cast<const Soul&>(*this);
	return eternity().utter(self.id(), saying);
}


matter::Training Spirit::exercise(const id::Word word, std::vector<Exercise> exercises) const
{
	return eternity().train(word, std::move(exercises));
}


void Spirit::contemplate(const Place& place) const
{
	const auto& self = static_cast<const Soul&>(*this);
	horizon().contemplate(self, place);
}


void Spirit::entrust(matter::Supplication kept)
{
	horizon().keep(std::move(kept));
}


void Spirit::repose() const
{
	const auto& self = static_cast<const Soul&>(*this);
	horizon().repose(self);
}


std::shared_ptr<const Contemplation> Spirit::contemplation() const
{
	const auto& self = static_cast<const Soul&>(*this);
	return horizon().contemplation(self.id());
}


} // namespace will::domain
