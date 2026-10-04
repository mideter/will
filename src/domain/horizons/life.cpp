#include "life.h"

#include "acts/creation.h"
#include "dimensions/eternity.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"
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


Life::Life(Spatiality& spatiality, Temporality& temporality)
	: spatiality_(spatiality)
	, temporality_(temporality)
{
	(void)Eternity::the();

	if (current_ != nullptr)
		throw std::logic_error("Only one Life");

	current_ = this;
}


Life::~Life()
{
	if (current_ == this)
		current_ = nullptr;
}


Creation Life::create()
{
	return Creation{Eternity::the(), spatiality_, temporality_};
}


std::vector<matter::Letter> Life::letters(const id::Place place) const
{
	const std::vector<matter::Placement> placed = spatiality_.placements(place, Spatiality::MaxLetterLimit);

	std::vector<id::Word> ids;
	ids.reserve(placed.size());
	for (const matter::Placement& row : placed)
		ids.push_back(row.id());

	std::vector<matter::Dating> dated = temporality_.datings(ids);
	std::sort(dated.begin(), dated.end(), [](const matter::Dating& a, const matter::Dating& b) {
		if (a.created_at().value() != b.created_at().value())
			return a.created_at().value() < b.created_at().value();
		return a.id() < b.id();
	});

	std::unordered_map<id::Word, matter::Word> uttered;
	for (matter::Word& row : Eternity::the().words(ids))
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
	for (matter::Execution& row : temporality_.executions(ids))
		deeds.emplace(row.id(), std::move(row));

	return deeds;
}


Life::Remembered Life::remembered(const id::Place place) const
{
	std::lock_guard lock(mutex_);

	const auto it = living_.find(place);
	if (it == living_.end())
		return {};

	Remembered out;
	out.whole = !it->second.empty();
	out.words.reserve(it->second.size());
	for (const std::weak_ptr<const Word>& word : it->second) {
		if (std::shared_ptr<const Word> alive = word.lock())
			out.words.push_back(std::move(alive));
		else
			out.whole = false;
	}

	return out;
}


void Life::remember(const id::Place place, const std::vector<std::shared_ptr<const Word>>& words)
{
	std::lock_guard lock(mutex_);
	living_[place].assign(words.begin(), words.end());
}


void Life::remember(const id::Place place, const std::shared_ptr<const Word>& word)
{
	std::lock_guard lock(mutex_);
	living_[place].push_back(word);
}


} // namespace will::domain
