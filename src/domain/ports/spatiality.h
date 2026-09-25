#pragma once

#include "acts/placement.h"
#include "beings/abode.h"
#include "identity/abode.h"
#include "identity/letter.h"
#include "identity/place.h"
#include "identity/soul.h"
#include "values/abode_name.h"

#include <cstdint>
#include <vector>


namespace will::domain {


/// Spatiality (Пространственность) — places and where a Word is fixed.
/// Living places are known to Space; this face keeps spatial material.
/// Place ids are pointed by Space; Spatiality remembers the high-water mark.
class Spatiality {
public:
	static constexpr std::uint32_t MaxLetterLimit = 1000;

	virtual ~Spatiality() = default;

	/// Next free place id (Space::point).
	virtual id::Place point() = 0;

	/// Raise the place high-water mark to at least this id.
	virtual void point(id::Place id) = 0;

	/// Abodes kept in space (id + name).
	virtual std::vector<Abode> abodes() = 0;

	/// Keep an abode in space (idempotent by id).
	virtual void keep(id::Abode id, AbodeName name) = 0;

	/// Record that a soul dwells in an abode (idempotent).
	virtual void join_abode(id::Abode abode, id::Soul soul) = 0;

	/// Fix a word in a place.
	virtual void place(id::Letter id, id::Place place) = 0;

	/// Placements in this place, newest first up to limit (caller dates/sorts for history).
	virtual std::vector<Placement> placements(id::Place place, std::uint32_t limit) const = 0;
};


} // namespace will::domain
