#pragma once

#include "identity/place.h"


namespace will::domain::matter {


/// The gates and the helm are born with the abode, its outer and its inner edge.
class Abode {
public:
	Abode(id::Place id, id::Place gates, id::Place helm);

	id::Place id() const noexcept { return id_; }
	id::Place gates() const noexcept { return gates_; }
	id::Place helm() const noexcept { return helm_; }

private:
	id::Place id_;
	id::Place gates_;
	id::Place helm_;
};


} // namespace will::domain::matter
