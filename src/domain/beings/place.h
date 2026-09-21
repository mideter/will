#pragma once

#include "identity/place.h"


namespace will::domain {


/// Place (Место) — where a Word may be fixed in time.
/// Abode is a place; a living Obedience/Shepherding pair is Tie (Узы).
class Place {
public:
	virtual ~Place() = default;

	Place(const Place&) = delete;
	Place& operator=(const Place&) = delete;

	id::Place id() const noexcept { return id_; }

protected:
	explicit Place(id::Place id) noexcept;
	Place(Place&& other) noexcept;
	Place& operator=(Place&& other) noexcept;

private:
	id::Place id_;
};


} // namespace will::domain
