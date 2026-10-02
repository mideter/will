#include "heaven.h"

#include "beings/immanents/abode.h"
#include "beings/immanents/soul.h"
#include "beings/immanents/witness.h"
#include "dimensions/eternity.h"

#include <memory>
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


std::vector<std::reference_wrapper<const Soul>> Heaven::contemplating(const Abode& abode) const
{
	std::lock_guard lock(mutex_);

	std::vector<std::reference_wrapper<const Soul>> out;
	for (const auto& [soul_id, contemplation] : contemplations_) {
		if (contemplation->abode().id() != abode.id())
			continue;
		const auto it = souls_.find(soul_id);
		if (it == souls_.end() || !it->second)
			continue;
		out.emplace_back(*it->second);
	}
	return out;
}


void Heaven::present(const Soul& soul)
{
	std::lock_guard lock(mutex_);
	souls_.insert_or_assign(soul.id(), &soul);
}


void Heaven::contemplate(const Soul& soul, const Abode& abode)
{
	std::lock_guard lock(mutex_);
	if (!souls_.contains(soul.id()))
		throw std::logic_error("Heaven does not know this soul");

	// The former gaze of this soul ends here.
	contemplations_.insert_or_assign(
		soul.id(),
		std::unique_ptr<Contemplation>(new Contemplation(static_cast<const Witness&>(soul), abode)));
}


const Contemplation& Heaven::contemplation(const id::Soul soul_id) const
{
	std::lock_guard lock(mutex_);

	const auto it = contemplations_.find(soul_id);
	if (it == contemplations_.end())
		throw std::logic_error("Heaven has no contemplation for this soul");

	return *it->second;
}


} // namespace will::domain
