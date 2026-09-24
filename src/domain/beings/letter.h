#pragma once

#include "acts/dating.h"
#include "acts/placement.h"
#include "acts/utterance.h"
#include "beings/place.h"
#include "beings/soul.h"
#include "identity/letter.h"
#include "values/timestamp.h"
#include "values/word.h"


namespace will::domain {


/// Letter (Письмо) — Word fixed in time in a Place; born from three faces.
/// Living place and author via Place::of / Soul::of.
class Letter : public Word {
public:
	/// Ids must match; place and author must be known to Space and Heaven.
	Letter(Utterance utterance, Placement placement, Dating dating);

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
