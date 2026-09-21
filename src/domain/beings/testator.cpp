#include "testator.h"

#include "acts/obedience.h"
#include "acts/tie.h"
#include "beings/soul.h"
#include "ports/temporality.h"

#include <algorithm>
#include <stdexcept>
#include <utility>


namespace will::domain {


Testator::Testator(Embodiment embodiment)
	: Novice(std::move(embodiment))
{}


void Testator::accept(const Supplication& ask) const
{
	ask.sign(*this);
}


void Testator::reject(const Supplication& ask) const
{
	ask.reject(*this);
}


Deed Testator::will(const Shepherding& shepherding, const Word& word) const
{
	if (shepherding.testator().Soul::id() != Soul::id())
		throw std::logic_error("not the testator of this shepherding");

	return temporality().will(dynamic_cast<const Obedience&>(shepherding),
							  static_cast<const Soul&>(*this), word);
}


const Shepherding& Testator::shepherding(const id::Obedience id) const
{
	const Tie& tie = Tie::of(id);
	if (tie.testator().Soul::id() != Soul::id())
		throw std::logic_error("not the testator of this shepherding");
	return tie;
}


const Supplication& Testator::supplication(const Novice& suppliant) const
{
	for (const auto& row : incoming_) {
		if (row->suppliant().Soul::id() == suppliant.Soul::id())
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


const Supplication& Testator::receive(Supplication supplication) const
{
	if (supplication.addressee().Soul::id() != Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");

	const id::Soul suppliant_id = supplication.suppliant().Soul::id();

	for (const auto& existing : incoming_) {
		if (existing->suppliant().Soul::id() == suppliant_id)
			return *existing;
	}

	incoming_.push_back(std::make_unique<Supplication>(std::move(supplication)));

	return *incoming_.back();
}


void Testator::drop_supplication(const Novice& suppliant) const
{
	const auto it = std::find_if(incoming_.begin(), incoming_.end(),
								 [&](const std::unique_ptr<Supplication>& row) {
									 return row->suppliant().Soul::id() == suppliant.Soul::id();
								 });

	if (it == incoming_.end())
		throw std::invalid_argument("unknown supplication");

	incoming_.erase(it);
}


} // namespace will::domain
