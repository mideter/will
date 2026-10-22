#pragma once

#include "identity/place.h"


namespace will::domain::matter {


/// Its part counts only for a cell: the gates and the helm are ever the cores of
/// theirs.
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
