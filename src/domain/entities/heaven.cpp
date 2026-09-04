#include "heaven.h"

#include "values/soul_name.h"

#include <utility>


namespace will::domain {


Heaven::Heaven(Eternity& eternity)
	: eternity_(eternity)
{
	for (Soul soul : eternity_.load_souls())
		insert(std::move(soul));
}


std::optional<Soul> Heaven::find_by_id(const id::Soul id) const
{
	std::lock_guard lock(mutex_);

	const auto it = souls_by_id_.find(id);
	if (it == souls_by_id_.end())
		return std::nullopt;

	return it->second;
}


ManBirth Heaven::birth_man(const DeviceToken& token)
{
	const SoulName name = SoulName::generate();
	ManBirth birth = eternity_.insert_man(token, name);
	insert(birth.soul);
	return birth;
}


std::vector<Vessel> Heaven::load_vessels() const
{
	return eternity_.load_vessels();
}


std::vector<Man> Heaven::load_men() const
{
	return eternity_.load_men();
}


void Heaven::insert(Soul soul)
{
	std::lock_guard lock(mutex_);
	souls_by_id_.insert_or_assign(soul.id(), std::move(soul));
}


} // namespace will::domain
