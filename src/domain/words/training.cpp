#include "training.h"

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
{}


Training::Training(const Birth<Life> birth, matter::Behest kept)
	: Behest(birth, kept)
	, exercises_(exercises_of(kept))
{}


} // namespace will::domain
