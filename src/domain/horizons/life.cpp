#include "life.h"

#include "acts/creation.h"
#include "dimensions/eternity.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"
#include "immanents/place.h"
#include "words/recollection.h"
#include "words/word.h"

#include <unordered_map>

#include <algorithm>
#include <stdexcept>
#include <utility>


namespace will::domain {


Life* Life::current_ = nullptr;


Life& Life::the()
{
	if (current_ == nullptr)
		throw std::logic_error("there is no Life");

	return *current_;
}


Life::Life()
{
	(void)horizon();

	if (current_ != nullptr)
		throw std::logic_error("Only one Life");

	current_ = this;
}


Life::~Life()
{
	// The World ends while Life is still there for its words to leave.
	creation_.reset();

	if (current_ == this)
		current_ = nullptr;
}


Eternity& Life::eternity() const
{
	return horizon();
}


Creation& Life::create()
{
	if (creation_ != nullptr)
		throw std::logic_error("Only one Creation");

	creation_ = std::make_unique<Creation>(Birth<Life>{*this});
	return *creation_;
}


Creation& Life::creation() const
{
	if (creation_ == nullptr)
		throw std::logic_error("there is no Creation");

	return *creation_;
}


Spatiality& Life::spatiality() const
{
	return creation().spatiality();
}


Temporality& Life::temporality() const
{
	return creation().temporality();
}


std::vector<matter::Letter> Life::letters(const id::Place place) const
{
	const std::vector<matter::Placement> placed = spatiality().placements(place, Spatiality::MaxLetterLimit);

	std::vector<id::Word> ids;
	ids.reserve(placed.size());
	for (const matter::Placement& row : placed)
		ids.push_back(row.id());

	std::vector<matter::Dating> dated = temporality().datings(ids);
	std::sort(dated.begin(), dated.end(), [](const matter::Dating& a, const matter::Dating& b) {
		if (a.created_at().value() != b.created_at().value())
			return a.created_at().value() < b.created_at().value();
		return a.id() < b.id();
	});

	std::unordered_map<id::Word, matter::Word> uttered;
	for (matter::Word& row : eternity().words(ids))
		uttered.emplace(row.id(), std::move(row));

	std::vector<matter::Letter> kept;
	kept.reserve(dated.size());
	for (matter::Dating& dating : dated) {
		const auto u = uttered.find(dating.id());
		if (u == uttered.end())
			continue;

		const id::Word word = dating.id();
		kept.emplace_back(std::move(u->second), matter::Placement{word, place}, std::move(dating));
	}

	return kept;
}


std::vector<matter::Behest> Life::behests(const id::Place place) const
{
	// The words of a tie are kept in the same parts as letters; those that are
	// deeds are named so by their execution.
	std::vector<matter::Letter> parts = letters(place);
	const std::unordered_map<id::Word, matter::Execution> deeds = executions_of(parts);

	std::vector<matter::Behest> kept;
	for (const matter::Letter& part : parts) {
		if (!deeds.contains(part.id()))
			kept.emplace_back(part.word(), part.placement(), part.dating());
	}

	return kept;
}


std::vector<matter::Deed> Life::deeds(const id::Place place) const
{
	std::vector<matter::Letter> parts = letters(place);
	const std::unordered_map<id::Word, matter::Execution> deeds = executions_of(parts);

	std::vector<matter::Deed> kept;
	for (const matter::Letter& part : parts) {
		if (const auto e = deeds.find(part.id()); e != deeds.end())
			kept.emplace_back(part.word(), part.placement(), part.dating(), e->second);
	}

	return kept;
}


std::unordered_map<id::Word, matter::Execution> Life::executions_of(const std::vector<matter::Letter>& parts) const
{
	std::vector<id::Word> ids;
	ids.reserve(parts.size());
	for (const matter::Letter& part : parts)
		ids.push_back(part.id());

	std::unordered_map<id::Word, matter::Execution> deeds;
	for (matter::Execution& row : temporality().executions(ids))
		deeds.emplace(row.id(), std::move(row));

	return deeds;
}


std::shared_ptr<const Recollection> Life::recollection(const Place& place) const
{
	std::lock_guard lock(mutex_);

	std::weak_ptr<const Recollection>& known = recollections_[place.id()];
	if (std::shared_ptr<const Recollection> held = known.lock())
		return held;

	auto recalled = std::make_shared<const Recollection>(Birth<Life>{*this}, place, place.recall(Birth<Life>{*this}));
	known = recalled;
	return recalled;
}


} // namespace will::domain
