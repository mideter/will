#pragma once

#include "identity/word.h"
#include "values/saying.h"
#include "properties/immanent.h"


namespace will::domain {


class Life;
class Soul;


/// What the spirit utters: a letter, a behest or a deed. It lives in the
/// recollection of its place, while that is held.
class Word : public Immanent<Life> {
public:
	virtual ~Word() = default;

	id::Word id() const noexcept { return id_; }
	const Saying& saying() const noexcept { return saying_; }

	/// A letter's writer, a behest's testator, a deed's novice.
	virtual const Soul& author() const = 0;

protected:
	Word(id::Word id, Saying saying);

private:
	id::Word id_;
	Saying saying_;
};


} // namespace will::domain
