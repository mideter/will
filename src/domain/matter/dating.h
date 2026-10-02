#pragma once

#include "identity/word.h"
#include "values/timestamp.h"


namespace will::domain::matter {


/// Dating — Word fixed in time; matter for a living Word (Letter, Deed).
/// Temporality keeps Datings.
class Dating {
public:
	Dating(id::Word id, Timestamp created_at);

	id::Word id() const noexcept { return id_; }
	Timestamp created_at() const noexcept { return created_at_; }

private:
	id::Word id_;
	Timestamp created_at_;
};


} // namespace will::domain::matter
