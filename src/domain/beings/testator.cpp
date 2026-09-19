#include "testator.h"

#include "beings/soul.h"
#include "ports/temporality.h"

#include <algorithm>
#include <stdexcept>
#include <utility>


namespace will::domain {


Testator::Testator(Embodiment embodiment)
	: Executor(std::move(embodiment))
{}


const Obedience& Testator::accept(const Supplication& supplication) const
{
	return supplication.consent(*this);
}


void Testator::refuse(const Supplication& supplication) const
{
	supplication.dismiss(*this);
}


Deed Testator::will(const Shepherding& shepherding, const Word& word) const
{
	if (shepherding.testator().Soul::id() != Soul::id())
		throw std::logic_error("not the testator of this shepherding");

	const Obedience face{shepherding.obedience_id(), shepherding.testator(), shepherding.executor()};
	return temporality().bequeath(face, static_cast<const Soul&>(*this), word);
}


const Shepherding& Testator::shepherding(const id::Obedience id) const
{
	for (const auto& place : shepherdings_) {
		if (place->obedience_id() == id)
			return *place;
	}
	throw std::invalid_argument("unknown shepherding");
}


const Supplication& Testator::supplication(const id::Supplication id) const
{
	for (const auto& row : incoming_) {
		if (row->id() == id)
			return *row;
	}
	throw std::invalid_argument("unknown supplication");
}


std::vector<std::reference_wrapper<const Supplication>> Testator::supplications() const
{
	std::vector<std::reference_wrapper<const Supplication>> out;
	out.reserve(incoming_.size());
	for (const auto& row : incoming_)
		out.emplace_back(*row);
	return out;
}


const Shepherding& Testator::keep(Shepherding place) const
{
	const id::Obedience id = place.obedience_id();
	for (const auto& existing : shepherdings_) {
		if (existing->obedience_id() == id)
			return *existing;
	}
	shepherdings_.push_back(std::make_unique<Shepherding>(std::move(place)));
	return *shepherdings_.back();
}


const Supplication& Testator::receive(Supplication supplication) const
{
	if (supplication.testator().id() != Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");
	if (supplication.status() != SupplicationStatus::pending)
		throw std::logic_error("supplication is not pending");

	const id::Supplication id = supplication.id();
	for (const auto& existing : incoming_) {
		if (existing->id() == id)
			return *existing;
	}
	incoming_.push_back(std::make_unique<Supplication>(std::move(supplication)));
	return *incoming_.back();
}


void Testator::drop_supplication(const id::Supplication id) const
{
	const auto it = std::find_if(incoming_.begin(), incoming_.end(),
								 [&](const std::unique_ptr<Supplication>& row) {
									 return row->id() == id;
								 });
	if (it == incoming_.end())
		throw std::invalid_argument("unknown supplication");
	incoming_.erase(it);
}


} // namespace will::domain
