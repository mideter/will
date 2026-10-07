#include "exercise.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Exercise::Exercise(std::string name, std::vector<Approach> approaches)
	: name_(std::move(name))
	, approaches_(std::move(approaches))
{
	if (name_.empty() || name_.size() > MaxNameLength)
		throw std::invalid_argument("Exercise name must be 1 to MaxNameLength bytes");
	if (approaches_.empty() || approaches_.size() > MaxApproaches)
		throw std::invalid_argument("Exercise needs 1 to MaxApproaches approaches");
}


} // namespace will::domain
