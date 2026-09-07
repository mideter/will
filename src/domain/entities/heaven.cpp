#include "heaven.h"

#include "values/soul_name.h"


namespace will::domain {


Heaven::Heaven(Eternity& eternity)
	: eternity_(eternity)
{}


std::optional<Soul> Heaven::find_by_id(const id::Soul id) const
{
	std::lock_guard lock(mutex_);

	const auto it = souls_by_id_.find(id);
	if (it == souls_by_id_.end())
		return std::nullopt;

	return *it->second;
}


Man Heaven::beget(const DeviceToken& token)
{
	const SoulName name = SoulName::generate();
	return eternity_.enroll(token, name);
}


std::vector<Man> Heaven::remember() const
{
	return eternity_.men();
}


void Heaven::index(const Soul& soul)
{
	std::lock_guard lock(mutex_);
	souls_by_id_.insert_or_assign(soul.id(), &soul);
}


} // namespace will::domain
