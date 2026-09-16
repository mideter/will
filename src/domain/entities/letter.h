#pragma once

#include "identity/letter.h"
#include "identity/place.h"
#include "identity/soul.h"
#include "values/timestamp.h"
#include "values/word.h"


namespace will::domain {


class Soul;


/// Letter (Письмо) — Word fixed in time in a Place.
/// Author is a soul known to Heaven; words without a living author are not retold.
class Letter : public Word {
public:
	/// author_id must be non-zero (enforced by id::Soul).
	Letter(id::Letter id, id::Place place_id, id::Soul author_id, Word word, Timestamp created_at);

	id::Letter id() const noexcept { return id_; }
	id::Place place_id() const noexcept { return place_id_; }
	id::Soul author_id() const noexcept { return author_id_; }
	/// Living author on Heaven. Throws if unknown.
	const Soul& author() const;
	Timestamp created_at() const noexcept { return created_at_; }

private:
	id::Letter id_;
	id::Place place_id_;
	id::Soul author_id_;
	Timestamp created_at_;
};


} // namespace will::domain
