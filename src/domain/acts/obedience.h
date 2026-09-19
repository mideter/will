#pragma once

#include "beings/place.h"
#include "identity/obedience.h"


namespace will::domain {


class Novice;
class Testator;


/// Obedience (Послушание) — Novice-facing interface of a shared Place:
/// ordered pair (testator, novice). Same place as Shepherding (Ведение);
/// one id. Where a Deed (Дело) is kept; born from accepted Supplication.
/// Owned on the heap by the Novice; also kept in Temporality.
/// Ending a place (secede) is deferred for now — all kept places are active.
class Obedience : public Place {
public:
	Obedience(id::Obedience id, const Testator& testator, const Novice& novice);
	Obedience(Obedience&& other) noexcept;
	Obedience& operator=(Obedience&&) = delete;
	Obedience(const Obedience&) = delete;
	Obedience& operator=(const Obedience&) = delete;

	/// Strongly typed obedience id (same value as Place::id()).
	id::Obedience obedience_id() const noexcept { return id::Obedience{id().value()}; }

	const Testator& testator() const noexcept { return testator_; }
	const Novice& novice() const noexcept { return novice_; }

private:
	const Testator& testator_;
	const Novice& novice_;
};


} // namespace will::domain
