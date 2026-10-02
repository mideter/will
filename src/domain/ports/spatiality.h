#pragma once

#include "matter/placement.h"
#include "matter/abode.h"
#include "matter/tie.h"
#include "identity/abode.h"
#include "identity/word.h"
#include "identity/place.h"
#include "identity/soul.h"
#include "values/abode_name.h"

#include <cstdint>
#include <optional>
#include <vector>


namespace will::domain {


/// Spatiality (Пространственность) — places and where a Word is fixed.
/// Living places are known to Space; Spatiality keeps spatial matter.
class Spatiality {
public:
	static constexpr std::uint32_t MaxLetterLimit = 1000;

	virtual ~Spatiality() = default;

	/// Abodes kept in space (id + name matter).
	virtual std::vector<matter::Abode> abodes() = 0;

	/// matter::Abode for this soul: restore if kept, otherwise open, keep, and join.
	virtual matter::Abode abide(id::Soul soul, AbodeName name) = 0;

	/// matter::Abode for the abode this soul already dwells in, if kept.
	virtual std::optional<matter::Abode> abode_of(id::Soul soul) const = 0;

	/// Keep an abode in space (idempotent by id).
	virtual void keep(id::Abode id, AbodeName name) = 0;

	/// Keep that a soul dwells in an abode (idempotent).
	virtual void join_abode(id::Abode abode, id::Soul soul) = 0;

	/// Keep the bond of a pair in a new point of Space.
	/// Refuses (std::logic_error) if the pair is already bound.
	virtual matter::Tie bind(id::Soul testator, id::Soul novice) = 0;

	/// All ties kept in space.
	virtual std::vector<matter::Tie> ties() const = 0;

	/// Fix a word in a place.
	virtual void place(id::Word id, id::Place place) = 0;

	/// matter::Placement of this word, if kept.
	virtual std::optional<matter::Placement> placement(id::Word id) const = 0;

	/// Placements in this place, newest first up to limit (caller dates/sorts for history).
	virtual std::vector<matter::Placement> placements(id::Place place, std::uint32_t limit) const = 0;
};


} // namespace will::domain
