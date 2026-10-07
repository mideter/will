#include "training.h"

#include <stdexcept>
#include <utility>


namespace will::domain::matter {


Training::Training(const id::Word word, std::vector<Exercise> exercises)
	: word_(word)
	, exercises_(std::move(exercises))
{
	if (exercises_.empty() || exercises_.size() > MaxExercises)
		throw std::invalid_argument("Training needs 1 to MaxExercises exercises");
}


} // namespace will::domain::matter
