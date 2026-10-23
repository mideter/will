#include "life.h"

#include "horizons/creation.h"
#include "dimensions/eternity.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"
#include "places/place.h"
#include "places/tie.h"
#include "words/behest.h"
#include "words/deed.h"
#include "words/training.h"
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


std::unordered_map<id::Word, std::vector<matter::Remark>> Life::remarks_on(const std::vector<id::Word>& trainings) const
{
	std::vector<matter::Remarked> marks = eternity().remarked(trainings);
	std::vector<id::Word> said;
	said.reserve(marks.size());
	for (const matter::Remarked& mark : marks)
		said.push_back(mark.id());

	std::unordered_map<id::Word, matter::Word> words;
	for (matter::Word& row : eternity().words(said))
		words.emplace(row.id(), std::move(row));
	std::unordered_map<id::Word, matter::Dating> datings;
	for (matter::Dating& row : temporality().datings(said))
		datings.emplace(row.id(), std::move(row));

	std::unordered_map<id::Word, std::vector<matter::Remark>> out;
	for (const matter::Remarked& mark : marks) {
		const auto w = words.find(mark.id());
		const auto d = datings.find(mark.id());
		if (w == words.end() || d == datings.end())
			continue;
		out[mark.training()].emplace_back(std::move(w->second), mark, std::move(d->second));
	}
	for (auto& [training, remarks] : out) {
		std::sort(remarks.begin(), remarks.end(), [](const matter::Remark& a, const matter::Remark& b) {
			if (a.dating().created_at().value() != b.dating().created_at().value())
				return a.dating().created_at().value() < b.dating().created_at().value();
			return a.id() < b.id();
		});
	}
	return out;
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

	// A word of a tie that fulfils a behest is a deed, reviewed or not yet; the others there are behests.
	std::unordered_map<id::Word, matter::Execution> executions;
	for (matter::Execution& row : temporality().executions(ids))
		executions.emplace(row.id(), std::move(row));
	std::unordered_map<id::Word, matter::Review> reviewed;
	for (matter::Review& row : temporality().reviews(ids))
		reviewed.emplace(row.id(), std::move(row));

	const bool in_tie = dynamic_cast<const Tie*>(&place) != nullptr;

	// The exercises of the words of a tie: what a training wills, what a deed has
	// done; and the approaches of the trainings done so far.
	std::unordered_map<id::Word, matter::Training> trained;
	std::unordered_map<id::Word, std::vector<matter::Effort>> exerted;
	std::unordered_map<id::Word, matter::Underway> underway;
	std::unordered_map<id::Word, std::vector<matter::Remark>> remarked;
	if (in_tie) {
		for (matter::Training& row : eternity().trainings(ids))
			trained.emplace(row.id(), std::move(row));
		for (matter::Effort& row : temporality().efforts(ids))
			exerted[row.training()].push_back(std::move(row));
		for (matter::Underway& row : temporality().underways(ids))
			underway.emplace(row.training(), std::move(row));
		remarked = remarks_on(ids);
	}
	const auto training_of = [&](const id::Word id) -> std::optional<matter::Training> {
		const auto t = trained.find(id);
		if (t == trained.end())
			return std::nullopt;
		return std::move(t->second);
	};

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
			std::optional<matter::Review> review;
			if (const auto r = reviewed.find(dating.id()); r != reviewed.end())
				review = std::move(r->second);
			words.push_back(std::make_shared<const Deed>(
				Birth<Life>{*this},
				matter::Deed{std::move(u->second), std::move(placement), std::move(dating), std::move(e->second),
							 training_of(dating.id()), std::move(review)}));
		} else {
			std::optional<matter::Training> training = training_of(dating.id());
			std::vector<matter::Effort> efforts;
			std::optional<matter::Underway> doing;
			std::vector<matter::Remark> remarks;
			if (training) {
				if (const auto x = remarked.find(dating.id()); x != remarked.end())
					remarks = std::move(x->second);
				if (const auto x = exerted.find(dating.id()); x != exerted.end())
					efforts = std::move(x->second);
				if (const auto x = underway.find(dating.id()); x != underway.end())
					doing = std::move(x->second);
			}
			matter::Behest kept{std::move(u->second), std::move(placement), std::move(dating), std::move(training),
								std::move(efforts), std::move(doing), std::move(remarks)};
			if (kept.training())
				words.push_back(std::make_shared<const Training>(Birth<Life>{*this}, std::move(kept)));
			else
				words.push_back(std::make_shared<const Behest>(Birth<Life>{*this}, std::move(kept)));
		}
	}

	return words;
}


} // namespace will::domain
