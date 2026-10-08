#include "training.h"

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
{}


Training::Training(const Birth<Life> birth, matter::Behest kept)
	: Behest(birth, kept)
	, exercises_(exercises_of(kept))
	, efforts_(kept.efforts())
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


void Training::exert(const Birth<Novice> birth, matter::Effort kept) const
{
	if (birth.parent().Soul::id() != tie().novice().Soul::id())
		throw std::logic_error("only the novice of its tie does a training");
	if (kept.training() != id())
		throw std::logic_error("the effort is of another training");

	std::lock_guard lock(*mutex_);
	efforts_.push_back(std::move(kept));
}


} // namespace will::domain
