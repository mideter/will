#pragma once

#include "matter/placement.h"
#include "matter/abode.h"
#include "matter/dweller.h"
#include "matter/room.h"
#include "matter/tie.h"
#include "identity/word.h"
#include "identity/place.h"
#include "identity/soul.h"

#include <cstdint>
#include <optional>
#include <vector>


namespace will::domain {


/// Places and where a word is fixed: abodes, dwellers, rooms, ties, placements. A host
/// keeps one abode, its gates and its helm marked with it. A dweller's kind kept anew
/// replaces the former one. An abode has one room for each place it reflects, furnished
/// inward. A pair is bound once. A place's placements come all. What would break
/// these it refuses; an unknown room it does not arrange.
class Spatiality {
public:
	virtual ~Spatiality() = default;

	virtual std::optional<matter::Abode> abode(id::Soul host) const = 0;

	virtual matter::Abode abide(id::Soul host) = 0;

	virtual matter::Dweller dwell(id::Place abode, id::Soul soul, matter::Dweller::Kind kind) = 0;

	virtual std::vector<matter::Dweller> dwellers() const = 0;

	virtual matter::Room furnish(id::Place abode, id::Place reflects) = 0;

	virtual matter::Room arrange(id::Place room, matter::Room::Part part) = 0;

	virtual std::vector<matter::Room> rooms(id::Place abode) const = 0;

	virtual matter::Tie bind(id::Soul testator, id::Soul novice) = 0;

	virtual std::vector<matter::Tie> ties() const = 0;

	virtual matter::Placement place(id::Word id, id::Place place) = 0;

	virtual std::optional<matter::Placement> placement(id::Word id) const = 0;

	virtual std::vector<matter::Placement> placements(id::Place place) const = 0;
};


} // namespace will::domain
