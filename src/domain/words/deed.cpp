#include "deed.h"

#include "men/man.h"
#include "places/place.h"
#include "places/tie.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Deed::Deed(Birth<Tie>, const Behest& behest, matter::Deed kept)
	: Deed(behest, std::move(kept))
{}


Deed::Deed(Birth<Life>, const Behest& behest, matter::Deed kept)
	: Deed(behest, std::move(kept))
{}


Deed::Deed(const Behest& behest, matter::Deed kept)
	: Word(kept.id(), kept.word().saying())
	, behest_(behest)
	, performed_(kept.performed() ? kept.performed()->exercises() : std::vector<Exercise>{})
	, created_at_(kept.dating().created_at())
	, review_(kept.review())
{
	if (kept.execution().behest() != behest_.id())
		throw std::invalid_argument("deed must fulfil the behest it is born with");
	if (kept.placement().place() != tie().id())
		throw std::invalid_argument("deed must be placed in the tie of its behest");
	if (kept.word().author() != tie().novice().Soul::id())
		throw std::invalid_argument("deed must be uttered by the novice of its tie");
}


const Soul& Deed::author() const
{
	return tie().novice();
}


std::optional<matter::Review> Deed::review() const
{
	std::lock_guard lock(*mutex_);
	return review_;
}


void Deed::review(const Birth<Testator> birth, matter::Review kept) const
{
	if (birth.parent().Soul::id() != tie().testator().Soul::id())
		throw std::logic_error("only the testator of the tie reviews its deed");
	if (kept.id() != id())
		throw std::logic_error("the review is of another deed");

	std::lock_guard lock(*mutex_);
	if (review_)
		throw std::logic_error("the deed is already reviewed");
	review_ = std::move(kept);
}


} // namespace will::domain
