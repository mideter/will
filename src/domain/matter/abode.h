#pragma once

#include "identity/place.h"
#include "values/abode_name.h"


namespace will::domain::matter {


/// The gates and the helm are born with the abode, the edges of its parts.
class Abode {
public:
	Abode(id::Place id, AbodeName name, id::Place gates, id::Place helm);

	id::Place id() const noexcept { return id_; }
	const AbodeName& name() const noexcept { return name_; }
	id::Place gates() const noexcept { return gates_; }
	id::Place helm() const noexcept { return helm_; }

private:
	id::Place id_;
	AbodeName name_;
	id::Place gates_;
	id::Place helm_;
};


} // namespace will::domain::matter
