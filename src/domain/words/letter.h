#pragma once

#include "matter/letter.h"
#include "immanents/place.h"
#include "immanents/soul.h"
#include "words/word.h"
#include "values/timestamp.h"


namespace will::domain {


/// Letter (Письмо) — Word fixed in time in a Place; born from matter::Letter.
/// Living place and author via Place::of / Soul::of.
class Letter : public Word {
public:
	/// Place and author must be known to Space and Heaven.
	explicit Letter(matter::Letter kept);

	const Place& place() const noexcept { return place_; }
	const Soul& author() const noexcept { return author_; }
	Timestamp created_at() const noexcept { return created_at_; }

private:
	const Place& place_;
	const Soul& author_;
	Timestamp created_at_;
};


} // namespace will::domain
