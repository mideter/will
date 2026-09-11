#include "heaven.h"

#include "entities/soul.h"
#include "ports/eternity.h"

#include <stdexcept>


namespace will::domain {


Eternity* Heaven::eternity_ = nullptr;
std::mutex Heaven::mutex_;
std::unordered_map<id::Soul, const Soul*> Heaven::souls_by_id_;


Heaven::Heaven() noexcept = default;


Heaven::Heaven(Eternity& eternity)
	: owns_pole_(true)
{
	std::lock_guard lock(mutex_);
	if (eternity_ != nullptr)
		throw std::logic_error("Only one World");
	eternity_ = &eternity;
}


Heaven::~Heaven()
{
	if (owns_pole_)
		clear_pole();
}


Heaven::Heaven(const Heaven&) noexcept
	: owns_pole_(false)
{}


Heaven& Heaven::operator=(const Heaven&) noexcept
{
	return *this;
}


Heaven::Heaven(Heaven&& other) noexcept
	: owns_pole_(std::exchange(other.owns_pole_, false))
{}


Heaven& Heaven::operator=(Heaven&& other) noexcept
{
	if (this == &other)
		return *this;
	if (owns_pole_)
		clear_pole();
	owns_pole_ = std::exchange(other.owns_pole_, false);
	return *this;
}


void Heaven::clear_pole() noexcept
{
	std::lock_guard lock(mutex_);
	souls_by_id_.clear();
	eternity_ = nullptr;
	owns_pole_ = false;
}


Eternity& Heaven::eternity()
{
	if (eternity_ == nullptr)
		throw std::logic_error("Heaven is not raised");
	return *eternity_;
}


const Eternity& Heaven::eternity() const
{
	if (eternity_ == nullptr)
		throw std::logic_error("Heaven is not raised");
	return *eternity_;
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


void Heaven::index(const Soul& soul)
{
	std::lock_guard lock(mutex_);
	souls_by_id_.insert_or_assign(soul.id(), &soul);
}


} // namespace will::domain
