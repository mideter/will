#pragma once

#include "identity/place.h"


namespace will::domain::matter {


/// Room (Комната) — a window in an Abode onto a place it reflects: the Abode
/// itself (the cell of one's own records) or a Tie one is a side of. It stands
/// in the inner part of the Abode, seen by the host and his friends, or in the
/// outer one, seen by his neighbours too. Spatiality keeps it.
class Room {
public:
	enum class Part {
		Inner,
		Outer,
	};

	Room(id::Place id, id::Place abode, id::Place reflects, Part part);

	id::Place id() const noexcept { return id_; }
	id::Place abode() const noexcept { return abode_; }
	id::Place reflects() const noexcept { return reflects_; }
	Part part() const noexcept { return part_; }

private:
	id::Place id_;
	id::Place abode_;
	id::Place reflects_;
	Part part_;
};


} // namespace will::domain::matter
