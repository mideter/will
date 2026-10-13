#pragma once

#include "identity/place.h"
#include "values/abode_name.h"


namespace will::domain::matter {


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
