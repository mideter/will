#include "witness.h"

#include "acts/dating.h"
#include "acts/placement.h"
#include "acts/utterance.h"
#include "beings/space.h"
#include "ports/eternity.h"
#include "ports/spatiality.h"
#include "ports/temporality.h"
#include "values/abode_name.h"

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


namespace will::domain {


Witness::Witness(Embodiment embodiment)
	: Man(std::move(embodiment))
{
	if (const std::optional<Abode> kept = spatiality().abode_of(Soul::id())) {
		abode_ = std::make_unique<Abode>(kept->abode_id(), kept->name());
	} else {
		abode_ = std::make_unique<Abode>(
			id::Abode{eternity().space().point().value()},
			AbodeName{std::string{Soul::name().text()}});
		spatiality().keep(abode_->abode_id(), abode_->name());
		spatiality().join_abode(abode_->abode_id(), Soul::id());
	}

	abode_->present();
	abode_->admit(*this);
}


void Witness::say(const Word& word) const
{
	if (!abode_->dwells(*this))
		throw std::logic_error("Witness does not dwell in the observed abode");

	const Utterance uttered = eternity().utter(Soul::id(), word);
	spatiality().place(uttered.id(), abode_->id());
	temporality().date(uttered.id(), eternity().time().instant());
}


std::vector<Letter> Witness::retell(const std::uint32_t limit) const
{
	if (limit == 0)
		throw std::invalid_argument("History limit must be positive");

	const std::uint32_t capped = std::min(limit, Spatiality::MaxLetterLimit);
	std::vector<Placement> placed = spatiality().placements(abode_->id(), Spatiality::MaxLetterLimit);

	std::vector<id::Letter> ids;
	ids.reserve(placed.size());
	std::unordered_map<id::Letter, id::Place> place_by_id;
	for (const Placement& row : placed) {
		ids.push_back(row.id());
		place_by_id.emplace(row.id(), row.place());
	}

	std::vector<Dating> dated = temporality().datings(ids);
	std::sort(dated.begin(), dated.end(), [](const Dating& a, const Dating& b) {
		return a.created_at().value() < b.created_at().value();
	});
	if (dated.size() > capped)
		dated.erase(dated.begin(), dated.end() - static_cast<std::ptrdiff_t>(capped));

	std::vector<id::Letter> selected;
	selected.reserve(dated.size());
	for (const Dating& row : dated)
		selected.push_back(row.id());

	std::vector<Utterance> uttered = eternity().utterances(selected);
	std::unordered_map<id::Letter, Utterance> utterance_by_id;
	for (Utterance& row : uttered)
		utterance_by_id.emplace(row.id(), std::move(row));

	std::unordered_map<id::Letter, Dating> dating_by_id;
	for (Dating& row : dated)
		dating_by_id.emplace(row.id(), std::move(row));

	std::vector<Letter> living;
	living.reserve(selected.size());
	for (const id::Letter id : selected) {
		const auto u = utterance_by_id.find(id);
		const auto d = dating_by_id.find(id);
		const auto p = place_by_id.find(id);
		if (u == utterance_by_id.end() || d == dating_by_id.end() || p == place_by_id.end())
			continue;
		if (!heaven().knows(u->second.author()))
			continue;
		living.push_back(Letter{std::move(u->second), Placement{id, p->second}, std::move(d->second)});
	}

	return living;
}


} // namespace will::domain
