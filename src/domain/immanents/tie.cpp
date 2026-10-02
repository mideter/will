#include "tie.h"

#include "words/deed.h"
#include "immanents/soul.h"
#include "horizons/space.h"
#include "immanents/testator.h"
#include "matter/deed.h"
#include "matter/execution.h"
#include "dimensions/temporality.h"
#include "properties/immanent.h"

#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>


namespace will::domain {


Tie::Tie(matter::Tie kept)
	: Place(id::Place{kept.id().value()})
	, Obedience(Soul::of(kept.testator()))
	, Shepherding(Soul::of(kept.novice()))
{
	Immanent<Space>::present<Place>();
}


Tie::~Tie()
{
	testator_.release(*this);
}


const Testator& Tie::testator() const
{
	return testator_;
}


const Novice& Tie::novice() const
{
	return novice_;
}


std::vector<Deed> Tie::deeds(const Novice& asker) const
{
	if (asker.Soul::id() != novice_.Soul::id() && asker.Soul::id() != testator_.Soul::id())
		throw std::logic_error("not a side of this tie");

	std::vector<Parts> kept = words(asker);

	std::vector<id::Word> ids;
	ids.reserve(kept.size());
	for (const Parts& parts : kept)
		ids.push_back(parts.utterance.id());

	std::unordered_map<id::Word, matter::Execution> executed;
	for (matter::Execution& row : asker.temporality().executions(ids))
		executed.emplace(row.id(), std::move(row));

	std::vector<Deed> shown;
	shown.reserve(kept.size());
	for (Parts& parts : kept) {
		std::optional<matter::Execution> execution;
		if (const auto e = executed.find(parts.utterance.id()); e != executed.end())
			execution = e->second;

		shown.emplace_back(matter::Deed{std::move(parts.utterance), std::move(parts.placement),
										std::move(parts.dating), std::move(execution)});
	}

	return shown;
}


} // namespace will::domain
