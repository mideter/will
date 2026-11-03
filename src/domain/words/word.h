#pragma once

#include "identity/word.h"
#include "values/saying.h"
#include "properties/immanent.h"


namespace will::domain {


class Life;
class Soul;


class Word : public Immanent<Life> {
public:
	virtual ~Word() = default;

	id::Word id() const noexcept { return id_; }
	const Saying& saying() const noexcept { return saying_; }

	virtual const Soul& author() const = 0;

protected:
	Word(id::Word id, Saying saying);

private:
	id::Word id_;
	Saying saying_;
};


} // namespace will::domain
