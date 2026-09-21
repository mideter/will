#pragma once

#include "beings/place.h"
#include "identity/obedience.h"


namespace will::domain {


class Novice;
class Testator;


/// Shepherding (Ведение) — Testator-facing interface of the shared Place.
/// Living heap object is Tie (Узы), owned by the Novice as Obedience.
/// Ending a place (secede) is deferred for now — all kept places are active.
class Shepherding : public virtual Place {
public:
	~Shepherding() override = default;

	Shepherding(const Shepherding&) = delete;
	Shepherding& operator=(const Shepherding&) = delete;

	id::Obedience obedience_id() const noexcept { return id::Obedience{id().value()}; }

	virtual const Testator& testator() const = 0;
	virtual const Novice& novice() const = 0;

protected:
	enum class AsTieFace : std::uint8_t { v };
	Shepherding(AsTieFace, id::Obedience id);
	Shepherding& operator=(Shepherding&&) = delete;
};


} // namespace will::domain
