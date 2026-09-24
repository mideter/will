#include "utterance.h"

#include <utility>


namespace will::domain {


Utterance::Utterance(const id::Letter id, const id::Soul author, Word word)
	: id_(id)
	, author_(author)
	, word_(std::move(word))
{}


} // namespace will::domain
