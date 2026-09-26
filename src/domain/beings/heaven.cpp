#include "heaven.h"

#include "beings/immanents/soul.h"
#include "ports/eternity.h"

#include <stdexcept>


namespace will::domain {


Heaven* Heaven::current_ = nullptr;


Heaven& Heaven::the()
{
	if (current_ == nullptr)
		throw std::logic_error("Heaven has not been brought forth");

	return *current_;
}


Heaven::Heaven(Eternity& eternity)
	: eternity_(eternity)
{
	if (current_ != nullptr)
		throw std::logic_error("Only one World");

	current_ = this;
}


Heaven::~Heaven()
{
	if (current_ == this)
		current_ = nullptr;
}


Eternity& Heaven::eternity()
{
	return eternity_;
}


const Eternity& Heaven::eternity() const
{
	return eternity_;
}


bool Heaven::knows(const id::Soul soul_id) const
{
	std::lock_guard lock(mutex_);
	return souls_.contains(soul_id);
}


const Soul& Heaven::soul(const id::Soul soul_id) const
{
	std::lock_guard lock(mutex_);

	const auto it = souls_.find(soul_id);
	if (it == souls_.end() || !it->second)
		throw std::logic_error("Heaven does not know this soul");

	return *it->second;
}


void Heaven::present(const Soul& soul)
{
	std::lock_guard lock(mutex_);
	souls_.insert_or_assign(soul.id(), &soul);
}


} // namespace will::domain
