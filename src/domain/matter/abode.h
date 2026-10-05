#pragma once

#include "identity/place.h"
#include "values/abode_name.h"


namespace will::domain::matter {


/// Matter of living Abode (Обитель) — abode id and name kept in Spatiality.
/// Not the living Abode (heap presence in Space).
class Abode {
public:
	Abode(id::Place id, AbodeName name);

	id::Place id() const noexcept { return id_; }
	const AbodeName& name() const noexcept { return name_; }

private:
	id::Place id_;
	AbodeName name_;
};


} // namespace will::domain::matter
