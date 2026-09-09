#include "abode.h"


namespace will::domain {


Abode::Abode(id::Abode id, AbodeName name)
	: id_(id)
	, name_(std::move(name))
{}


void Abode::admit(const Man& man)
{
	std::lock_guard lock(mutex_);
	dwellers_.insert(&man);
}


bool Abode::dwells(const Man& man) const
{
	std::lock_guard lock(mutex_);
	return dwellers_.contains(&man);
}


} // namespace will::domain
