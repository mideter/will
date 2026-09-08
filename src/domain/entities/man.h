#pragma once

#include "entities/soul.h"
#include "entities/vessel.h"
#include "identity/man.h"


namespace will::domain {


/// Man (Человек) — soul dwelling in a vessel. One object is both Soul and Vessel.
/// In Eternity: value snapshots (copyable). In the living World: polymorphic base of Witness.
class Man : public Soul, public Vessel {
public:
	Man(id::Man id, Soul&& soul, Vessel&& vessel);
	~Man() override = default;

	id::Man id() const noexcept { return id_; }

	bool operator==(const Man& other) const = default;

private:
	id::Man id_;
};


} // namespace will::domain
