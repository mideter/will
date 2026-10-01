#pragma once

#include "identity/word.h"
#include "identity/soul.h"
#include "values/saying.h"


namespace will::domain {


/// Utterance — Saying and author kept in Eternity; matter for a living Word (Letter, Deed).
class Utterance {
public:
	Utterance(id::Word id, id::Soul author, Saying saying);

	id::Word id() const noexcept { return id_; }
	id::Soul author() const noexcept { return author_; }
	const Saying& saying() const noexcept { return saying_; }

private:
	id::Word id_;
	id::Soul author_;
	Saying saying_;
};


} // namespace will::domain
