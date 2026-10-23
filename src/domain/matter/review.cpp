#include "review.h"


namespace will::domain::matter {


Review::Review(const id::Word deed, const Timestamp at)
	: deed_(deed)
	, at_(at)
{}


} // namespace will::domain::matter
