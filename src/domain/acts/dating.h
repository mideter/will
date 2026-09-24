#pragma once

#include "identity/letter.h"
#include "values/timestamp.h"


namespace will::domain {


/// Dating — Word fixed in time; material for living Letter. Temporality keeps Datings.
class Dating {
public:
	Dating(id::Letter id, Timestamp created_at);

	id::Letter id() const noexcept { return id_; }
	Timestamp created_at() const noexcept { return created_at_; }

private:
	id::Letter id_;
	Timestamp created_at_;
};


} // namespace will::domain
