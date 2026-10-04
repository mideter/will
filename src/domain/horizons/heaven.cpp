#include "heaven.h"

#include "immanents/abode.h"
#include "immanents/soul.h"
#include "immanents/witness.h"
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
	if (!knows(soul.id()))
		throw std::logic_error("Heaven does not know this soul");

	// Born and ended outside the lock: a gaze receives the letters it beholds,
	// which may read the dimensions and ask Heaven for their authors.
	std::shared_ptr<const Contemplation> gaze(new Contemplation(static_cast<const Witness&>(soul), abode));

	std::lock_guard lock(mutex_);
	std::swap(contemplations_[soul.id()], gaze);
}


void Heaven::cease(const Soul& soul)
{
	std::shared_ptr<const Contemplation> ended;

	std::lock_guard lock(mutex_);
	const auto it = contemplations_.find(soul.id());
	if (it == contemplations_.end())
		return;

	ended = std::move(it->second);
	contemplations_.erase(it);
}


std::vector<std::shared_ptr<const Contemplation>> Heaven::gazes(const Abode& abode) const
{
	std::lock_guard lock(mutex_);

	std::vector<std::shared_ptr<const Contemplation>> out;
	for (const auto& [soul_id, gaze] : contemplations_) {
		if (&gaze->abode() == &abode)
			out.push_back(gaze);
	}
	return out;
}


std::shared_ptr<const Contemplation> Heaven::contemplation(const id::Soul soul_id) const
{
	std::lock_guard lock(mutex_);

	const auto it = contemplations_.find(soul_id);
	if (it == contemplations_.end())
		return nullptr;

	return it->second;
}


} // namespace will::domain
