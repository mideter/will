#pragma once

#include "identity/place.h"
#include "matter/dating.h"
#include "matter/placement.h"
#include "matter/word.h"
#include "properties/immanent.h"

#include <vector>


namespace will::domain {


class Space;
class Witness;


/// Place (Место) — where a Word may be fixed in time; immanent to Space.
/// Abode is a place; a living Obedience/Shepherding pair is Tie (Узы).
/// Living places are known to Space; Place::of looks them up.
/// A place reaches no dimension by itself: its words are read through the
/// one who asks for them.
class Place : public Immanent<Space> {
public:
	virtual ~Place() = default;

	/// Throws if unknown.
	static const Place& of(id::Place id);

	id::Place id() const noexcept { return id_; }

protected:
	/// Parts of one word placed here, as the three dimensions keep them.
	struct Parts {
		matter::Word word;
		matter::Placement placement;
		matter::Dating dating;
	};

	/// Words placed here, oldest first (capped by Spatiality::MaxLetterLimit).
	std::vector<Parts> words(const Witness& asker) const;

	/// For virtual-base roles (Obedience/Shepherding) that are never most-derived;
	/// the living Tie supplies Place(id). Throws if actually invoked.
	Place();
	explicit Place(id::Place id) noexcept;

private:
	id::Place id_;
};


} // namespace will::domain
