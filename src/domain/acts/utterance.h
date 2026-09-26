#pragma once

#include "identity/letter.h"
#include "identity/soul.h"
#include "values/saying.h"


namespace will::domain {


/// Utterance — Saying and author kept in Eternity; material for living Letter.
class Utterance {
public:
	Utterance(id::Letter id, id::Soul author, Saying saying);

	id::Letter id() const noexcept { return id_; }
	id::Soul author() const noexcept { return author_; }
	const Saying& saying() const noexcept { return saying_; }

private:
	id::Letter id_;
	id::Soul author_;
	Saying saying_;
};


} // namespace will::domain
