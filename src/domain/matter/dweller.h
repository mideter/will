#pragma once

#include "identity/place.h"
#include "identity/soul.h"


namespace will::domain::matter {


/// Dweller (Обитатель) — a soul dwelling in another's Abode, and how its host
/// regards him: an acquaintance (знакомый) sees only that he dwells there; a
/// neighbour (ближний) sees the outer part; a friend (друг) sees all.
/// Spatiality keeps it; the host is not a dweller of his own Abode.
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
