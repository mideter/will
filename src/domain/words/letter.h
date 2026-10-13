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


/// Letter (Письмо) — a word its host wrote in his abode.
class Letter : public Word {
public:
	Letter(Birth<Abode>, matter::Letter kept);
	Letter(Birth<Life>, matter::Letter kept);

	const Place& place() const noexcept { return place_; }
	const Soul& author() const noexcept override { return author_; }
	Timestamp created_at() const noexcept { return created_at_; }

private:
	explicit Letter(matter::Letter kept);

	const Place& place_;
	const Soul& author_;
	Timestamp created_at_;
};


} // namespace will::domain
