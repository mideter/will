#pragma once

#include "identity/letter.h"
#include "identity/soul.h"
#include "values/word.h"


namespace will::domain {


/// Utterance — Word and author kept in Eternity; material for living Letter.
class Utterance {
public:
	Utterance(id::Letter id, id::Soul author, Word word);

	id::Letter id() const noexcept { return id_; }
	id::Soul author() const noexcept { return author_; }
	const Word& word() const noexcept { return word_; }

private:
	id::Letter id_;
	id::Soul author_;
	Word word_;
};


} // namespace will::domain
