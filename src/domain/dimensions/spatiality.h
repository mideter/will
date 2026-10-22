#pragma once

#include "matter/placement.h"
#include "matter/abode.h"
#include "matter/dweller.h"
#include "matter/room.h"
#include "matter/tie.h"
#include "identity/word.h"
#include "identity/place.h"
#include "identity/soul.h"
#include "values/abode_name.h"

#include <cstdint>
#include <optional>
#include <vector>


namespace will::domain {


/// Places and where a word is fixed: abodes,
/// dwellers, rooms, ties, placements.
class Spatiality {
public:
	static constexpr std::uint32_t MaxLetterLimit = 1000;

	virtual ~Spatiality() = default;

	virtual std::optional<matter::Abode> abode(id::Soul host) const = 0;

	/// Refuses (std::logic_error) if the host already keeps one.
	virtual matter::Abode abide(id::Soul host, AbodeName name) = 0;

	/// A kind kept anew replaces the former one.
	virtual matter::Dweller dwell(id::Place abode, id::Soul soul, matter::Dweller::Kind kind) = 0;

	virtual std::vector<matter::Dweller> dwellers() const = 0;

	/// In the inner part. Refuses (std::logic_error) if the abode already has a room
	/// reflecting this aspect of this place.
	virtual matter::Room furnish(id::Place abode, id::Place reflects, matter::Room::Aspect aspect) = 0;

	/// Throws (std::invalid_argument) if the room is unknown.
	virtual matter::Room arrange(id::Place room, matter::Room::Part part) = 0;

	virtual std::vector<matter::Room> rooms(id::Place abode) const = 0;

	/// Refuses (std::logic_error) if the pair is already bound.
	virtual matter::Tie bind(id::Soul testator, id::Soul novice) = 0;

	virtual std::vector<matter::Tie> ties() const = 0;

	virtual matter::Placement place(id::Word id, id::Place place) = 0;

	virtual std::optional<matter::Placement> placement(id::Word id) const = 0;

	/// Newest first, up to the limit.
	virtual std::vector<matter::Placement> placements(id::Place place, std::uint32_t limit) const = 0;
};


} // namespace will::domain
