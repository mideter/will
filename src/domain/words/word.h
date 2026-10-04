#pragma once

#include "identity/word.h"
#include "values/saying.h"
#include "properties/immanent.h"


namespace will::domain {


class Life;


/// Word (Слово) — that which the spirit utters; ontological role, immanent to
/// Life: it lives while it is held, and Life remembers it meanwhile. Its matter
/// is kept by the dimensions; its identity is given in Eternity. Neither copied
/// nor moved.
/// Holds what was said (Saying) but is not the text itself.
/// Living Letter and Deed are Word in the world.
class Word : public Immanent<Life> {
public:
	virtual ~Word() = default;

	id::Word id() const noexcept { return id_; }
	const Saying& saying() const noexcept { return saying_; }

protected:
	Word(id::Word id, Saying saying);

private:
	id::Word id_;
	Saying saying_;
};


} // namespace will::domain
