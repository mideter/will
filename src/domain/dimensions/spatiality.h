#pragma once

#include "matter/placement.h"
#include "matter/abode.h"
#include "matter/tie.h"
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

	/// The abode of this host, if kept.
	virtual std::optional<matter::Abode> abode(id::Soul host) const = 0;

	/// Keep a new abode of this host in a new point of Space.
	/// Refuses (std::logic_error) if the host already keeps one.
	virtual matter::Abode abide(id::Soul host, AbodeName name) = 0;

	/// Keep the bond of a pair in a new point of Space.
	/// Refuses (std::logic_error) if the pair is already bound.
	virtual matter::Tie bind(id::Soul testator, id::Soul novice) = 0;

	/// All ties kept in space.
	virtual std::vector<matter::Tie> ties() const = 0;

	/// Fix a word in a place.
	virtual matter::Placement place(id::Word id, id::Place place) = 0;

	/// matter::Placement of this word, if kept.
	virtual std::optional<matter::Placement> placement(id::Word id) const = 0;

	/// Placements in this place, newest first up to limit (caller dates/sorts for history).
	virtual std::vector<matter::Placement> placements(id::Place place, std::uint32_t limit) const = 0;
};


} // namespace will::domain
