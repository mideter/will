#pragma once

#include "beings/place.h"
#include "identity/obedience.h"


namespace will::domain {


class Novice;
class Testator;


/// Shepherding (Ведение) — Testator-facing interface of the shared Place
/// also seen as Obedience (Послушание). One id; owned on the heap by the
/// Testator; also kept in Temporality.
/// Ending a place (secede) is deferred for now — all kept places are active.
class Shepherding : public Place {
public:
	Shepherding(id::Obedience id, const Testator& testator, const Novice& novice);
	Shepherding(Shepherding&& other) noexcept;
	Shepherding& operator=(Shepherding&&) = delete;
	Shepherding(const Shepherding&) = delete;
	Shepherding& operator=(const Shepherding&) = delete;

	/// Shared place id (same value as the Obedience face).
	id::Obedience obedience_id() const noexcept { return id::Obedience{id().value()}; }

	const Testator& testator() const noexcept { return testator_; }
	const Novice& novice() const noexcept { return novice_; }

private:
	const Testator& testator_;
	const Novice& novice_;
};


} // namespace will::domain
