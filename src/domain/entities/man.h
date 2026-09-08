#pragma once

#include "entities/soul.h"
#include "entities/vessel.h"
#include "identity/man.h"

#include <string_view>


namespace will::domain {


/// Man (Человек) — soul dwelling in a vessel. One object is both Soul and Vessel.
/// In Eternity: value snapshots (copyable). In the living World: polymorphic base of Witness.
/// Speech (say) belongs to man; only a living Witness can have it heard and fixed.
class Man : public Soul, public Vessel {
public:
	Man(id::Man id, Soul&& soul, Vessel&& vessel);
	virtual ~Man() = default;

	id::Man id() const noexcept { return id_; }

	id::Soul soul_id() const noexcept { return Soul::id(); }
	id::Vessel vessel_id() const noexcept { return Vessel::id(); }

	/// Act of will: speak. Value men from Eternity cannot; Witness overrides.
	virtual void say(std::string_view body) const;

	bool operator==(const Man& other) const = default;

private:
	id::Man id_;
};


} // namespace will::domain
