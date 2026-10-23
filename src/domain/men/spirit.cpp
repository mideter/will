#include "spirit.h"

#include "men/soul.h"

#include <utility>


namespace will::domain {


matter::Word Spirit::utter(const Saying& saying) const
{
	const auto& self = static_cast<const Soul&>(*this);
	return eternity().utter(self.id(), saying);
}


matter::Training Spirit::utter(const id::Word word, std::vector<Exercise> exercises) const
{
	return eternity().utter(word, std::move(exercises));
}


matter::Remarked Spirit::remark(const id::Word remark, const id::Word training, const std::uint32_t exercise) const
{
	return eternity().remark(remark, training, exercise);
}


void Spirit::contemplate(const Place& place) const
{
	horizon().contemplate(*this, place);
}


void Spirit::ask(const Soul& addressee) const
{
	horizon().ask(*this, addressee);
}


void Spirit::repose() const
{
	horizon().repose(*this);
}


std::shared_ptr<const Contemplation> Spirit::contemplation() const
{
	return horizon().contemplation(*this);
}


} // namespace will::domain
