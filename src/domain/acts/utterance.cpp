#include "utterance.h"

#include <utility>


namespace will::domain {


Utterance::Utterance(const id::Letter id, const id::Soul author, Saying saying)
	: id_(id)
	, author_(author)
	, saying_(std::move(saying))
{}


} // namespace will::domain
