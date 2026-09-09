#pragma once

#include "identity/abode.h"
#include "identity/soul.h"
#include "identity/letter.h"
#include "values/timestamp.h"
#include "values/word.h"


namespace will::domain {


/// Letter (Письмо) — Word fixed in time in an abode.
class Letter : public Word {
public:
	/// author_id must be non-zero (enforced by id::Soul).
	Letter(id::Letter id, id::Abode abode_id, id::Soul author_id, Word word, Timestamp created_at);

	id::Letter id() const noexcept { return id_; }
	id::Abode abode_id() const noexcept { return abode_id_; }
	id::Soul author_id() const noexcept { return author_id_; }
	Timestamp created_at() const noexcept { return created_at_; }

private:
	id::Letter id_;
	id::Abode abode_id_;
	id::Soul author_id_;
	Timestamp created_at_;
};


} // namespace will::domain
