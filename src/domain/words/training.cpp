#include "training.h"

#include "men/man.h"
#include "men/novice.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


std::vector<Exercise> Training::exercises_of(const matter::Behest& kept)
{
	if (!kept.training())
		throw std::invalid_argument("a training wills exercises");
	return kept.training()->exercises();
}


Training::Training(const Birth<Tie> birth, matter::Behest kept)
	: Behest(birth, kept)
	, exercises_(exercises_of(kept))
	, efforts_(kept.efforts())
	, underway_(kept.underway())
{}


Training::Training(const Birth<Life> birth, matter::Behest kept)
	: Behest(birth, kept)
	, exercises_(exercises_of(kept))
	, efforts_(kept.efforts())
	, underway_(kept.underway())
{}


std::vector<matter::Effort> Training::efforts() const
{
	std::lock_guard lock(*mutex_);
	return efforts_;
}


bool Training::done(const std::uint32_t exercise, const std::uint32_t approach) const
{
	std::lock_guard lock(*mutex_);
	for (const matter::Effort& effort : efforts_) {
		if (effort.exercise() == exercise && effort.approach() == approach)
			return true;
	}
	return false;
}


std::optional<matter::Underway> Training::underway() const
{
	std::lock_guard lock(*mutex_);
	return underway_;
}


void Training::begin(const Birth<Novice> birth, matter::Underway kept) const
{
	if (birth.parent().Soul::id() != tie().novice().Soul::id())
		throw std::logic_error("only the novice of the tie begins an approach of its training");
	if (kept.training() != id())
		throw std::logic_error("the approach is of another training");

	std::lock_guard lock(*mutex_);
	if (underway_)
		throw std::logic_error("another approach of the training is underway");
	underway_ = std::move(kept);
}


void Training::exert(const Birth<Novice> birth, matter::Effort kept) const
{
	if (birth.parent().Soul::id() != tie().novice().Soul::id())
		throw std::logic_error("only the novice of the tie exerts in its training");
	if (kept.training() != id())
		throw std::logic_error("the effort is of another training");

	std::lock_guard lock(*mutex_);
	efforts_.push_back(std::move(kept));
	underway_.reset();
}


} // namespace will::domain
