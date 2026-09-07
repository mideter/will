#include "heaven.h"

#include "values/soul_name.h"

#include <stdexcept>


namespace will::domain {


Heaven::Heaven(Eternity& eternity)
	: eternity_(eternity)
{}


bool Heaven::knows(const id::Soul id) const
{
	std::lock_guard lock(mutex_);
	return souls_by_id_.contains(id);
}


const Soul& Heaven::soul(const id::Soul id) const
{
	std::lock_guard lock(mutex_);

	const auto it = souls_by_id_.find(id);
	if (it == souls_by_id_.end() || !it->second)
		throw std::logic_error("Heaven does not know this soul");

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
