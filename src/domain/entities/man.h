#pragma once

#include "entities/soul.h"
#include "entities/vessel.h"
#include "identity/man.h"


namespace will::domain {


/// Man (Человек) — soul dwelling in a vessel. One object is both Soul and Vessel.
class Man : public Soul, public Vessel {
public:
	Man(id::Man id, Soul&& soul, Vessel&& vessel);

	id::Man id() const noexcept { return id_; }

	id::Soul soul_id() const noexcept { return Soul::id(); }
	id::Vessel vessel_id() const noexcept { return Vessel::id(); }

	bool operator==(const Man&) const = default;

private:
	id::Man id_;
};


} // namespace will::domain
