#pragma once

#include "identity/letter.h"
#include "identity/place.h"
#include "identity/soul.h"
#include "values/timestamp.h"
#include "values/word.h"


namespace will::domain {


/// Inscription (Начертание) — Word recorded in time in a Place.
/// Material for living Letter; not the Letter itself. Temporality keeps Inscriptions.
class Inscription {
public:
	Inscription(id::Letter id, id::Place place, id::Soul author, Word word, Timestamp created_at);

	id::Letter id() const noexcept { return id_; }
	id::Place place() const noexcept { return place_; }
	id::Soul author() const noexcept { return author_; }
	const Word& word() const noexcept { return word_; }
	Timestamp created_at() const noexcept { return created_at_; }

private:
	id::Letter id_;
	id::Place place_;
	id::Soul author_;
	Word word_;
	Timestamp created_at_;
};


} // namespace will::domain
