#pragma once

#include "matter/letter.h"
#include "beings/immanents/place.h"
#include "beings/immanents/soul.h"
#include "beings/word.h"
#include "identity/word.h"
#include "values/saying.h"
#include "values/timestamp.h"


namespace will::domain {


/// Letter (Письмо) — Word fixed in time in a Place; born from matter::Letter.
/// Living place and author via Place::of / Soul::of.
class Letter : public Word {
public:
	/// Place and author must be known to Space and Heaven.
	explicit Letter(matter::Letter kept);

	id::Word id() const noexcept { return id_; }
	const Place& place() const noexcept { return place_; }
	const Soul& author() const noexcept { return author_; }
	const Saying& saying() const noexcept { return saying_; }
	Timestamp created_at() const noexcept { return created_at_; }

private:
	id::Word id_;
	const Place& place_;
	const Soul& author_;
	Saying saying_;
	Timestamp created_at_;
};


} // namespace will::domain
