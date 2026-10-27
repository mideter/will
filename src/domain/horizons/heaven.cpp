#include "heaven.h"

#include "men/soul.h"
#include "men/testator.h"
#include "relations/supplication.h"
#include "men/man.h"
#include "places/gates.h"
#include "relations/knock.h"
#include "dimensions/eternity.h"

#include <algorithm>
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


std::vector<std::reference_wrapper<const Man>> Heaven::contemplating(const Place& place) const
{
	std::lock_guard lock(mutex_);

	std::vector<std::reference_wrapper<const Man>> out;
	for (const auto& [eye, contemplation] : contemplations_) {
		if (&contemplation->place() == &place)
			out.emplace_back(contemplation->who());
	}
	return out;
}


void Heaven::present(const Soul& soul)
{
	std::lock_guard lock(mutex_);
	souls_.insert_or_assign(soul.id(), &soul);
}


void Heaven::contemplate(const Spirit& eye, const Place& place)
{
	const Man& man = dynamic_cast<const Man&>(eye);
	if (!knows(man.Soul::id()))
		throw std::logic_error("Heaven does not know this soul");

	// Born and ended outside the lock: a gaze holds the recollection of its
	// place, which may read the dimensions and ask Heaven for their authors.
	std::shared_ptr<const Contemplation> gaze = std::make_shared<const Contemplation>(Birth<Heaven>{*this}, man, place);

	std::lock_guard lock(mutex_);
	std::swap(contemplations_[&eye], gaze);
}


void Heaven::knock(const Birth<Gates> gates, const Contemplation& gaze)
{
	std::shared_ptr<const Contemplation> knock = std::make_shared<const Knock>(Birth<Heaven>{*this}, gaze.who(), gates.parent());

	std::lock_guard lock(mutex_);
	const auto it = contemplations_.find(&static_cast<const Spirit&>(gaze.who()));
	if (it == contemplations_.end() || it->second.get() != &gaze)
		throw std::logic_error("this gaze has turned away");

	std::swap(it->second, knock);
}


std::vector<std::reference_wrapper<const Man>> Heaven::knocking(const Gates& gates) const
{
	std::lock_guard lock(mutex_);

	std::vector<std::reference_wrapper<const Man>> out;
	for (const auto& [eye, contemplation] : contemplations_) {
		if (&contemplation->place() == static_cast<const Place*>(&gates) && dynamic_cast<const Knock*>(contemplation.get()))
			out.emplace_back(contemplation->who());
	}
	return out;
}


void Heaven::repose(const Spirit& eye)
{
	std::shared_ptr<const Contemplation> ended;

	std::lock_guard lock(mutex_);
	const auto it = contemplations_.find(&eye);
	if (it == contemplations_.end())
		return;

	ended = std::move(it->second);
	contemplations_.erase(it);
}


std::shared_ptr<const Supplication> Heaven::keep(matter::Supplication kept)
{
	auto supplication = std::make_shared<const Supplication>(Birth<Heaven>{*this}, std::move(kept));

	std::lock_guard lock(mutex_);
	supplications_.push_back(supplication);
	return supplication;
}


void Heaven::release(const Supplication& answered)
{
	std::shared_ptr<const Supplication> released;

	std::lock_guard lock(mutex_);
	const auto it = std::find_if(supplications_.begin(), supplications_.end(),
								 [&](const auto& kept) { return kept.get() == &answered; });
	if (it == supplications_.end())
		return;

	released = std::move(*it);
	supplications_.erase(it);
}


std::vector<std::shared_ptr<const Supplication>> Heaven::supplications(const id::Soul addressee) const
{
	std::lock_guard lock(mutex_);

	std::vector<std::shared_ptr<const Supplication>> out;
	for (const std::shared_ptr<const Supplication>& kept : supplications_) {
		if (kept->addressee().Soul::id() == addressee)
			out.push_back(kept);
	}
	return out;
}


std::shared_ptr<const Contemplation> Heaven::contemplation(const Spirit& eye) const
{
	std::lock_guard lock(mutex_);

	const auto it = contemplations_.find(&eye);
	if (it == contemplations_.end())
		return nullptr;

	return it->second;
}


} // namespace will::domain
