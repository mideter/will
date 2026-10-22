#pragma once

#include "identity/place.h"
#include "identity/soul.h"


namespace will::domain::matter {


/// One dwelling in another's abode as its host regards him:
/// знакомый, ближний or друг.
class Dweller {
public:
	enum class Kind {
		Acquaintance,
		Neighbour,
		Friend,
	};

	Dweller(id::Place abode, id::Soul soul, Kind kind);

	id::Place abode() const noexcept { return abode_; }
	id::Soul soul() const noexcept { return soul_; }
	Kind kind() const noexcept { return kind_; }

private:
	id::Place abode_;
	id::Soul soul_;
	Kind kind_;
};


} // namespace will::domain::matter
