#include "abode.h"

#include <utility>


namespace will::domain {


Abode::Abode(id::Abode id, AbodeName name)
	: id_(id)
	, name_(std::move(name))
	, mutex_(std::make_unique<std::mutex>())
{}


Abode::Abode(Abode&& other) noexcept
	: id_(other.id_)
	, name_(std::move(other.name_))
	, mutex_(std::move(other.mutex_))
	, dwellers_(std::move(other.dwellers_))
{}


Abode& Abode::operator=(Abode&& other) noexcept
{
	if (this == &other)
		return *this;
	id_ = other.id_;
	name_ = std::move(other.name_);
	mutex_ = std::move(other.mutex_);
	dwellers_ = std::move(other.dwellers_);
	return *this;
}


void Abode::admit(const Man& man)
{
	std::lock_guard lock(*mutex_);
	dwellers_.insert(&man);
}


bool Abode::dwells(const Man& man) const
{
	std::lock_guard lock(*mutex_);
	return dwellers_.contains(&man);
}


} // namespace will::domain
