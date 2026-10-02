#include "utterance.h"

#include <utility>


namespace will::domain::matter {


Utterance::Utterance(const id::Word id, const id::Soul author, Saying saying)
	: id_(id)
	, author_(author)
	, saying_(std::move(saying))
{}


} // namespace will::domain::matter
