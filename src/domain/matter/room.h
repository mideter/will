#pragma once

#include "identity/place.h"


namespace will::domain::matter {


/// Room (Комната) — a window in an Abode onto what it reflects: the words of a
/// place (the Abode itself — the cell of one's own records — or a Tie one is a
/// side of), the threshold of the Abode (the Gates), its dwellers (the
/// upper room — Горница) or the unborn (the birth room — Родильная). It stands in the inner part of the Abode, seen by the
/// host and his friends, or in the outer one, seen by his neighbours too.
/// Spatiality keeps it.
class Room {
public:
	enum class Part {
		Inner,
		Outer,
	};

	/// What a room reflects of the place it looks onto.
	enum class Aspect {
		Words,     // the words of the place
		Threshold, // the threshold of the abode: who stands at its gates
		Dwellers,  // the dwellers of the abode
		Birth,     // the unborn: bodies awaiting their birth
	};

	Room(id::Place id, id::Place abode, id::Place reflects, Aspect aspect, Part part);

	id::Place id() const noexcept { return id_; }
	id::Place abode() const noexcept { return abode_; }
	id::Place reflects() const noexcept { return reflects_; }
	Aspect aspect() const noexcept { return aspect_; }
	Part part() const noexcept { return part_; }

private:
	id::Place id_;
	id::Place abode_;
	id::Place reflects_;
	Aspect aspect_;
	Part part_;
};


} // namespace will::domain::matter
