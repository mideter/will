#include "witness.h"

#include "acts/dating.h"
#include "acts/placement.h"
#include "acts/utterance.h"
#include "beings/immanents/abode.h"
#include "ports/spatiality.h"
#include "ports/temporality.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>


namespace will::domain {


Witness::Witness(Embodiment embodiment)
	: Man(std::move(embodiment))
{
	contemplate(abode());
}


void Witness::contemplate(const Abode& abode) const
{
	Spirit::contemplate(abode);
}


void Witness::say(const Saying& saying) const
{
	const Abode& place = contemplation().abode();
	if (!place.dwells(*this))
		throw std::logic_error("Witness does not dwell in the contemplated abode");

	const Utterance uttered = utter(saying);
	spatiality().place(uttered.id(), place.id());
	temporality().date(uttered.id());
}


std::vector<Letter> Witness::retell(const std::uint32_t limit) const
{
	if (limit == 0)
		throw std::invalid_argument("History limit must be positive");

	const Abode& place = contemplation().abode();
	const std::uint32_t capped = std::min(limit, Spatiality::MaxLetterLimit);
	std::vector<Placement> placed = spatiality().placements(place.id(), Spatiality::MaxLetterLimit);

	std::vector<id::Word> ids;
	ids.reserve(placed.size());
	std::unordered_map<id::Word, id::Place> place_by_id;
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

	std::vector<id::Word> selected;
	selected.reserve(dated.size());
	for (const Dating& row : dated)
		selected.push_back(row.id());

	std::vector<Utterance> uttered = utterances(selected);
	std::unordered_map<id::Word, Utterance> utterance_by_id;
	for (Utterance& row : uttered)
		utterance_by_id.emplace(row.id(), std::move(row));

	std::unordered_map<id::Word, Dating> dating_by_id;
	for (Dating& row : dated)
		dating_by_id.emplace(row.id(), std::move(row));

	std::vector<Letter> living;
	living.reserve(selected.size());
	for (const id::Word id : selected) {
		const auto u = utterance_by_id.find(id);
		const auto d = dating_by_id.find(id);
		const auto p = place_by_id.find(id);
		if (u == utterance_by_id.end() || d == dating_by_id.end() || p == place_by_id.end())
			continue;
		if (!knows(u->second.author()))
			continue;
		living.push_back(Letter{std::move(u->second), Placement{id, p->second}, std::move(d->second)});
	}

	return living;
}


} // namespace will::domain
