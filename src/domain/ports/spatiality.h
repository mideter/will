#pragma once

#include "acts/placement.h"
#include "acts/abiding.h"
#include "identity/abode.h"
#include "identity/letter.h"
#include "identity/place.h"
#include "identity/soul.h"
#include "values/abode_name.h"

#include <cstdint>
#include <optional>
#include <vector>


namespace will::domain {


/// Spatiality (Пространственность) — places and where a Word is fixed.
/// Living places are known to Space; Spatiality keeps spatial material.
class Spatiality {
public:
	static constexpr std::uint32_t MaxLetterLimit = 1000;

	virtual ~Spatiality() = default;

	/// Abodes kept in space (id + name material).
	virtual std::vector<Abiding> abodes() = 0;

	/// Abiding for the abode this soul already dwells in, if kept.
	virtual std::optional<Abiding> abode_of(id::Soul soul) const = 0;

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
