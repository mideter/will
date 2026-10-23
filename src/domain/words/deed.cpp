#include "deed.h"

#include "men/man.h"
#include "places/place.h"
#include "places/tie.h"

#include <stdexcept>
#include <utility>


namespace will::domain {
namespace {


const Tie& living_tie(const id::Place id)
{
	const auto* const tie = dynamic_cast<const Tie*>(&Place::of(id));
	if (!tie)
		throw std::invalid_argument("deed must be placed in a tie");

	return *tie;
}


} // namespace


Deed::Deed(Birth<Tie>, matter::Deed kept)
	: Deed(std::move(kept))
{}


Deed::Deed(Birth<Life>, matter::Deed kept)
	: Deed(std::move(kept))
{}


Deed::Deed(matter::Deed kept)
	: Word(kept.id(), kept.word().saying())
	, tie_(living_tie(kept.placement().place()))
	, behest_(kept.execution().behest())
	, performed_(kept.performed() ? kept.performed()->exercises() : std::vector<Exercise>{})
	, created_at_(kept.dating().created_at())
	, review_(kept.review())
{
	if (kept.word().author() != tie_.novice().Soul::id())
		throw std::invalid_argument("deed must be uttered by the novice of its tie");
}


const Soul& Deed::author() const
{
	return tie_.novice();
}


std::optional<matter::Review> Deed::review() const
{
	std::lock_guard lock(*mutex_);
	return review_;
}


void Deed::review(const Birth<Testator> birth, matter::Review kept) const
{
	if (birth.parent().Soul::id() != tie_.testator().Soul::id())
		throw std::logic_error("only the testator of the tie reviews its deed");
	if (kept.id() != id())
		throw std::logic_error("the review is of another deed");

	std::lock_guard lock(*mutex_);
	if (review_)
		throw std::logic_error("the deed is already reviewed");
	review_ = std::move(kept);
}


} // namespace will::domain
