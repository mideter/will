#include "word.h"

#include <utility>


namespace will::domain {


Word::Word(const id::Word id, Saying saying)
	: id_(id)
	, saying_(std::move(saying))
{}


} // namespace will::domain
