#include "heaven.h"

#include "values/soul_name.h"

#include <stdexcept>


namespace will::domain {


Eternity* Heaven::eternity_ = nullptr;
std::mutex Heaven::mutex_;
std::unordered_map<id::Soul, const Soul*> Heaven::souls_by_id_;


Heaven::Heaven(Eternity& eternity)
{
	std::lock_guard lock(mutex_);
	if (eternity_ != nullptr)
		throw std::logic_error("Only one World");
	eternity_ = &eternity;
}


Heaven::~Heaven()
{
	std::lock_guard lock(mutex_);
	souls_by_id_.clear();
	eternity_ = nullptr;
}


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
	return eternity_->enroll(token, name);
}


std::vector<Man> Heaven::remember() const
{
	return eternity_->men();
}


void Heaven::index(const Soul& soul)
{
	std::lock_guard lock(mutex_);
	souls_by_id_.insert_or_assign(soul.id(), &soul);
}


} // namespace will::domain
