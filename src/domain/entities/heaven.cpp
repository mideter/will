#include "heaven.h"

#include "entities/soul.h"
#include "ports/eternity.h"

#include <stdexcept>


namespace will::domain {


Eternity* Heaven::eternity_ = nullptr;
std::mutex Heaven::mutex_;
std::unordered_map<id::Soul, const Soul*> Heaven::souls_;


Heaven::Heaven() noexcept = default;


Heaven::Heaven(Eternity& eternity)
{
	std::lock_guard lock(mutex_);
	if (eternity_ != nullptr)
		throw std::logic_error("Only one World");
	eternity_ = &eternity;
}


void Heaven::roll() noexcept
{
	std::lock_guard lock(mutex_);
	souls_.clear();
	eternity_ = nullptr;
}


Eternity& Heaven::eternity()
{
	if (eternity_ == nullptr)
		throw std::logic_error("Heaven has not been brought forth");
	return *eternity_;
}


const Eternity& Heaven::eternity() const
{
	if (eternity_ == nullptr)
		throw std::logic_error("Heaven has not been brought forth");
	return *eternity_;
}


bool Heaven::knows(const id::Soul id) const
{
	std::lock_guard lock(mutex_);
	return souls_.contains(id);
}


const Soul& Heaven::soul(const id::Soul id) const
{
	std::lock_guard lock(mutex_);

	const auto it = souls_.find(id);
	if (it == souls_.end() || !it->second)
		throw std::logic_error("Heaven does not know this soul");

	return *it->second;
}


void Heaven::index(const Soul& soul)
{
	std::lock_guard lock(mutex_);
	souls_.insert_or_assign(soul.id(), &soul);
}


} // namespace will::domain
