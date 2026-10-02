#include "place.h"

#include "immanents/witness.h"
#include "horizons/space.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <utility>


namespace will::domain {


Place::Place()
	: id_(id::Place{1})
{
	throw std::logic_error("Place default ctor is only for virtual-base faces");
}


Place::Place(const id::Place id) noexcept
	: id_(id)
{}


const Place& Place::of(const id::Place id)
{
	return Space::the().place(id);
}


std::vector<Place::Parts> Place::words(const Witness& asker) const
{
	const std::vector<matter::Placement> placed =
		asker.spatiality().placements(id(), Spatiality::MaxLetterLimit);

	std::vector<id::Word> ids;
	ids.reserve(placed.size());
	for (const matter::Placement& row : placed)
		ids.push_back(row.id());

	std::vector<matter::Dating> dated = asker.temporality().datings(ids);
	std::sort(dated.begin(), dated.end(), [](const matter::Dating& a, const matter::Dating& b) {
		if (a.created_at().value() != b.created_at().value())
			return a.created_at().value() < b.created_at().value();
		return a.id() < b.id();
	});

	std::unordered_map<id::Word, matter::Utterance> uttered;
	for (matter::Utterance& row : asker.utterances(ids))
		uttered.emplace(row.id(), std::move(row));

	std::vector<Parts> kept;
	kept.reserve(dated.size());
	for (matter::Dating& dating : dated) {
		const auto u = uttered.find(dating.id());
		if (u == uttered.end())
			continue;

		const id::Word word = dating.id();
		kept.push_back(Parts{std::move(u->second), matter::Placement{word, id()}, std::move(dating)});
	}

	return kept;
}


} // namespace will::domain
