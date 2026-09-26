#pragma once

#include "identity/place.h"
#include "properties/immanent.h"


namespace will::domain {


class Space;


/// Place (Место) — where a Word may be fixed in time; immanent to Space.
/// Abode is a place; a living Obedience/Shepherding pair is Tie (Узы).
/// Living places are known to Space; Place::of looks them up.
class Place : public Immanent<Space> {
public:
	virtual ~Place() = default;

	/// Living place known to Space. Throws if unknown.
	static const Place& of(id::Place id);

	id::Place id() const noexcept { return id_; }

protected:
	friend class Witness;

	/// For virtual-base roles (Obedience/Shepherding) that are never most-derived;
	/// the living Tie supplies Place(id). Throws if actually invoked.
	Place();
	explicit Place(id::Place id) noexcept;

private:
	id::Place id_;
};


} // namespace will::domain
