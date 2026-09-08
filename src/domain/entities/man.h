#pragma once

#include "entities/soul.h"
#include "entities/vessel.h"
#include "identity/man.h"


namespace will::domain {


class Abode;
class Shelter;


/// Man (Человек) — soul dwelling in a vessel. One object is both Soul and Vessel.
/// May hold an optional Shelter* (focus of the soul); never owned here.
class Man : public Soul, public Vessel {
public:
	Man(id::Man id, Soul&& soul, Vessel&& vessel);

	id::Man id() const noexcept { return id_; }

	id::Soul soul_id() const noexcept { return Soul::id(); }
	id::Vessel vessel_id() const noexcept { return Vessel::id(); }

	/// Present cover of the soul, if any (owned by an Abode).
	Shelter* shelter() const noexcept { return shelter_; }

	bool operator==(const Man& other) const
	{
		return id_ == other.id_ && Soul::operator==(other) && Vessel::operator==(other);
	}

private:
	friend class Abode;

	void bind_shelter(Shelter* shelter) noexcept { shelter_ = shelter; }

	id::Man id_;
	Shelter* shelter_ = nullptr;
};


} // namespace will::domain
