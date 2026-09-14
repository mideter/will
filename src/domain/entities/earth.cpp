#include "earth.h"

#include "entities/abode.h"
#include "entities/vessel.h"
#include "ports/temporality.h"

#include <stdexcept>


namespace will::domain {


Temporality* Earth::temporality_ = nullptr;
std::mutex Earth::mutex_;
std::unordered_map<id::Vessel, const Vessel*> Earth::vessels_;
std::unordered_map<DeviceToken, id::Vessel> Earth::id_by_token_;
std::unordered_map<id::Abode, std::unique_ptr<Abode>> Earth::abodes_;


Earth::Earth() noexcept = default;


Earth::Earth(Temporality& temporality)
	: owns_pole_(true)
{
	std::lock_guard lock(mutex_);
	if (temporality_ != nullptr)
		throw std::logic_error("Only one World");
	temporality_ = &temporality;

	for (Abode& place : temporality.abodes()) {
		const id::Abode id = place.id();
		abodes_.emplace(id, std::make_unique<Abode>(std::move(place)));
	}

	if (!abodes_.contains(id::Abode::global()))
		throw std::logic_error("Earth requires the global abode");
}


Earth::~Earth()
{
	if (owns_pole_)
		clear_pole();
}


Earth::Earth(const Earth&) noexcept
	: owns_pole_(false)
{}


Earth& Earth::operator=(const Earth&) noexcept
{
	return *this;
}


Earth::Earth(Earth&& other) noexcept
	: owns_pole_(std::exchange(other.owns_pole_, false))
{}


Earth& Earth::operator=(Earth&& other) noexcept
{
	if (this == &other)
		return *this;
	if (owns_pole_)
		clear_pole();
	owns_pole_ = std::exchange(other.owns_pole_, false);
	return *this;
}


void Earth::clear_pole() noexcept
{
	std::lock_guard lock(mutex_);
	vessels_.clear();
	id_by_token_.clear();
	abodes_.clear();
	temporality_ = nullptr;
	owns_pole_ = false;
}


Temporality& Earth::temporality()
{
	if (temporality_ == nullptr)
		throw std::logic_error("Earth is not raised");
	return *temporality_;
}


const Temporality& Earth::temporality() const
{
	if (temporality_ == nullptr)
		throw std::logic_error("Earth is not raised");
	return *temporality_;
}


bool Earth::knows(const id::Vessel id) const
{
	std::lock_guard lock(mutex_);
	return vessels_.contains(id);
}


bool Earth::knows(const id::Abode id) const
{
	std::lock_guard lock(mutex_);
	return abodes_.contains(id);
}


Abode& Earth::abode(const id::Abode id)
{
	std::lock_guard lock(mutex_);
	const auto it = abodes_.find(id);
	if (it == abodes_.end() || !it->second)
		throw std::logic_error("Earth does not know this abode");
	return *it->second;
}


const Abode& Earth::abode(const id::Abode id) const
{
	std::lock_guard lock(mutex_);
	const auto it = abodes_.find(id);
	if (it == abodes_.end() || !it->second)
		throw std::logic_error("Earth does not know this abode");
	return *it->second;
}


const Vessel& Earth::vessel(const id::Vessel id) const
{
	std::lock_guard lock(mutex_);

	const auto it = vessels_.find(id);
	if (it == vessels_.end() || !it->second)
		throw std::logic_error("Earth does not know this vessel");

	return *it->second;
}


void Earth::fix(const id::Abode abode, const id::Soul author, const Word& word) const
{
	temporality().fix(abode, author, word);
}


std::optional<id::Vessel> Earth::id_of(const DeviceToken& token) const
{
	std::lock_guard lock(mutex_);

	const auto it = id_by_token_.find(token);
	if (it == id_by_token_.end())
		return std::nullopt;

	return it->second;
}


void Earth::index(const Vessel& vessel)
{
	std::lock_guard lock(mutex_);
	id_by_token_.insert_or_assign(vessel.token(), vessel.id());
	vessels_.insert_or_assign(vessel.id(), &vessel);
}


void Earth::index(Abode place)
{
	const id::Abode id = place.id();
	std::lock_guard lock(mutex_);
	abodes_.insert_or_assign(id, std::make_unique<Abode>(std::move(place)));
}


} // namespace will::domain
