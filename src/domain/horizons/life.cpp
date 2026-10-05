#include "life.h"

#include "acts/creation.h"
#include "dimensions/eternity.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"
#include "immanents/place.h"
#include "immanents/tie.h"
#include "words/behest.h"
#include "words/deed.h"
#include "words/letter.h"
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


std::shared_ptr<const Recollection> Life::recollection(const Birth<Place> birth)
{
	return the().recollection(birth.parent());
}


std::shared_ptr<const Recollection> Life::recollection(const Place& place) const
{
	std::lock_guard lock(mutex_);

	std::weak_ptr<const Recollection>& known = recollections_[place.id()];
	if (std::shared_ptr<const Recollection> held = known.lock())
		return held;

	auto recalled = std::make_shared<const Recollection>(Birth<Life>{*this}, place, recall(place));
	known = recalled;
	return recalled;
}


std::vector<std::shared_ptr<const Word>> Life::recall(const Place& place) const
{
	const std::vector<matter::Placement> placed = spatiality().placements(place.id(), Spatiality::MaxLetterLimit);

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

	// A word of a tie that fulfils a behest is a deed; the others there are behests.
	std::unordered_map<id::Word, matter::Execution> executions;
	for (matter::Execution& row : temporality().executions(ids))
		executions.emplace(row.id(), std::move(row));

	const bool in_tie = dynamic_cast<const Tie*>(&place) != nullptr;

	std::vector<std::shared_ptr<const Word>> words;
	words.reserve(dated.size());
	for (matter::Dating& dating : dated) {
		const auto u = uttered.find(dating.id());
		if (u == uttered.end())
			continue;

		matter::Placement placement{dating.id(), place.id()};
		if (!in_tie) {
			words.push_back(std::make_shared<const Letter>(
				Birth<Life>{*this}, matter::Letter{std::move(u->second), std::move(placement), std::move(dating)}));
		} else if (const auto e = executions.find(dating.id()); e != executions.end()) {
			words.push_back(std::make_shared<const Deed>(
				Birth<Life>{*this},
				matter::Deed{std::move(u->second), std::move(placement), std::move(dating), std::move(e->second)}));
		} else {
			words.push_back(std::make_shared<const Behest>(
				Birth<Life>{*this}, matter::Behest{std::move(u->second), std::move(placement), std::move(dating)}));
		}
	}

	return words;
}


} // namespace will::domain
