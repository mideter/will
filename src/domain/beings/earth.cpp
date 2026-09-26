#include "earth.h"

#include "beings/immanents/vessel.h"

#include <stdexcept>


namespace will::domain {


Earth* Earth::current_ = nullptr;


Earth& Earth::the()
{
	if (current_ == nullptr)
		throw std::logic_error("Earth has not been brought forth");

	return *current_;
}


Earth::Earth(Temporality& temporality, Spatiality& spatiality)
	: temporality_(temporality)
	, spatiality_(spatiality)
{
	if (current_ != nullptr)
		throw std::logic_error("Only one World");

	current_ = this;
}


Earth::~Earth()
{
	if (current_ == this)
		current_ = nullptr;
}


bool Earth::knows(const id::Vessel id) const
{
	std::lock_guard lock(mutex_);
	return vessels_.contains(id);
}


const Vessel& Earth::vessel(const id::Vessel id) const
{
	std::lock_guard lock(mutex_);

	const auto it = vessels_.find(id);
	if (it == vessels_.end() || !it->second)
		throw std::logic_error("Earth does not know this vessel");

	return *it->second;
}


std::optional<id::Vessel> Earth::id_of(const DeviceToken& token) const
{
	std::lock_guard lock(mutex_);

	const auto it = id_by_token_.find(token);
	if (it == id_by_token_.end())
		return std::nullopt;

	return it->second;
}


void Earth::present(const Vessel& vessel)
{
	std::lock_guard lock(mutex_);
	id_by_token_.insert_or_assign(vessel.token(), vessel.id());
	vessels_.insert_or_assign(vessel.id(), &vessel);
}


} // namespace will::domain
