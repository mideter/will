#pragma once

#include "matter/letter.h"
#include "properties/birth.h"
#include "places/place.h"
#include "men/soul.h"
#include "words/word.h"
#include "values/timestamp.h"


namespace will::domain {


class Abode;
class Life;


/// Letter (Письмо) — Word fixed in time in a Place; born from matter::Letter.
/// Living place and author via Place::of / Soul::of.
class Letter : public Word {
public:
	/// Said now, born of its abode; or recalled from memory, born of Life.
	/// Place and author must be known to Space and Heaven.
	Letter(Birth<Abode>, matter::Letter kept);
	Letter(Birth<Life>, matter::Letter kept);

	const Place& place() const noexcept { return place_; }
	const Soul& author() const noexcept { return author_; }
	Timestamp created_at() const noexcept { return created_at_; }

private:
	explicit Letter(matter::Letter kept);

	const Place& place_;
	const Soul& author_;
	Timestamp created_at_;
};


} // namespace will::domain
