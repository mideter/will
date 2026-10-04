#pragma once

#include "identity/word.h"
#include "identity/soul.h"
#include "values/saying.h"


namespace will::domain::matter {


/// Matter of living Word (Слово) — its id, author and Saying, kept in Eternity.
/// The eternal part of the matter of every Letter and Deed.
class Word {
public:
	Word(id::Word id, id::Soul author, Saying saying);

	id::Word id() const noexcept { return id_; }
	id::Soul author() const noexcept { return author_; }
	const Saying& saying() const noexcept { return saying_; }

private:
	id::Word id_;
	id::Soul author_;
	Saying saying_;
};


} // namespace will::domain::matter
