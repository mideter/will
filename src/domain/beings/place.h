#pragma once

#include "identity/place.h"


namespace will::domain {


/// Place (Место) — where a Word may be fixed in time.
/// Abode is a place; a living Obedience/Shepherding pair is Tie (Узы).
/// Living places are known to Space; Place::of looks them up.
class Place {
public:
	virtual ~Place() = default;

	Place(const Place&) = delete;
	Place& operator=(const Place&) = delete;

	/// Living place known to Space. Throws if unknown.
	static const Place& of(id::Place id);

	id::Place id() const noexcept { return id_; }

protected:
	friend class Witness;

	/// For virtual-base faces (Obedience/Shepherding) that are never most-derived;
	/// the living Tie supplies Place(id). Throws if actually invoked.
	Place();
	explicit Place(id::Place id) noexcept;
	Place(Place&& other) noexcept;
	Place& operator=(Place&& other) noexcept;

	/// Present this living place to Space (heap-stable address).
	void present() const;

private:
	id::Place id_;
};


} // namespace will::domain
