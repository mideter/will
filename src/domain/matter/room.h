#pragma once

#include "identity/place.h"


namespace will::domain::matter {


/// Room (Комната) — the inner part is seen by the host and his friends, the outer one by
/// his neighbours too.
class Room {
public:
	enum class Part {
		Inner,
		Outer,
	};

	enum class Aspect {
		Words,     // the words of the place
		Threshold, // the threshold of the abode: who stands at its gates
		Dwellers,  // the dwellers of the abode
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
