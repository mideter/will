#pragma once

#include "acts/inscription.h"
#include "beings/place.h"
#include "beings/soul.h"
#include "identity/letter.h"
#include "values/timestamp.h"
#include "values/word.h"


namespace will::domain {


/// Letter (Письмо) — Word fixed in time in a Place; born from Inscription.
/// Living place and author via Place::of / Soul::of; Temporality keeps Inscriptions.
class Letter : public Word {
public:
	/// Place and author must be known to Space and Heaven (throws if unknown).
	explicit Letter(Inscription inscription);

	id::Letter id() const noexcept { return id_; }
	const Place& place() const noexcept { return place_; }
	const Soul& author() const noexcept { return author_; }
	Timestamp created_at() const noexcept { return created_at_; }

private:
	id::Letter id_;
	const Place& place_;
	const Soul& author_;
	Timestamp created_at_;
};


} // namespace will::domain
