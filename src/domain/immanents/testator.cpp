#include "testator.h"

#include "matter/dating.h"
#include "matter/placement.h"
#include "matter/word.h"
#include "immanents/contemplation.h"
#include "immanents/soul.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"

#include <algorithm>
#include <stdexcept>
#include <utility>


namespace will::domain {


Testator::Testator(Birth<World>, matter::Man kept)
	: Novice(std::move(kept))
{}


const Shepherding& Testator::accept(const Supplication& ask) const
{
	return ask.sign(*this);
}


void Testator::reject(const Supplication& ask) const
{
	ask.reject(*this);
}


std::shared_ptr<const Behest> Testator::will(const Shepherding& shepherding, const Saying& saying) const
{
	if (shepherding.testator().Soul::id() != Soul::id())
		throw std::logic_error("not the testator of this shepherding");
	if (!contemplates(shepherding))
		throw std::logic_error("one wills only in the tie one contemplates");

	matter::Word uttered = utter(saying);
	matter::Placement placed = spatiality().place(uttered.id(), shepherding.id());
	matter::Dating dated = temporality().date(uttered.id());

	return shepherding.inscribe(
		Birth<Testator>{*this}, matter::Behest{std::move(uttered), std::move(placed), std::move(dated)});
}


const Shepherding& Testator::shepherding(const Novice& novice) const
{
	for (const Shepherding* place : shepherdings_) {
		if (place && place->novice().Soul::id() == novice.Soul::id())
			return *place;
	}

	throw std::invalid_argument("unknown shepherding");
}


const Shepherding& Testator::shepherd(const Shepherding& place) const
{
	if (place.testator().Soul::id() != Soul::id())
		throw std::logic_error("not the testator of this shepherding");

	for (const Shepherding* existing : shepherdings_) {
		if (existing && existing->id() == place.id())
			return *existing;
	}

	shepherdings_.push_back(&place);
	return place;
}


void Testator::release(const Shepherding& place) const
{
	const auto it = std::find(shepherdings_.begin(), shepherdings_.end(), &place);
	if (it != shepherdings_.end())
		shepherdings_.erase(it);
}


std::shared_ptr<const Supplication> Testator::supplication(const Novice& suppliant) const
{
	for (std::shared_ptr<const Supplication>& pending : supplications()) {
		if (pending->suppliant().Soul::id() == suppliant.Soul::id())
			return std::move(pending);
	}

	throw std::invalid_argument("unknown supplication");
}


std::vector<std::shared_ptr<const Supplication>> Testator::supplications() const
{
	return Spirit::supplications();
}


void Testator::hear(Birth<Novice> birth, matter::Supplication kept) const
{
	hold(std::make_shared<const Supplication>(birth, std::move(kept)));
}


void Testator::hear(Birth<World> birth, matter::Supplication kept) const
{
	hold(std::make_shared<const Supplication>(birth, std::move(kept)));
}


void Testator::drop(const Supplication& answered) const
{
	Spirit::release(answered);
}


void Testator::hold(std::shared_ptr<const Supplication> supplication) const
{
	if (supplication->addressee().Soul::id() != Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");

	const id::Soul suppliant_id = supplication->suppliant().Soul::id();

	for (const std::shared_ptr<const Supplication>& pending : supplications()) {
		if (pending->suppliant().Soul::id() == suppliant_id)
			return;
	}

	// A bound pair's supplication was answered by the bond itself.
	for (const Shepherding* place : shepherdings_) {
		if (place && place->novice().Soul::id() == suppliant_id)
			return;
	}

	keep(std::move(supplication));
}


} // namespace will::domain
