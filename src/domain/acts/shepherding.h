#pragma once

#include "beings/place.h"
#include "identity/obedience.h"


namespace will::domain {


class Soul;


/// Shepherding (Ведение) — Testator-facing interface of the shared Place
/// also seen as Obedience (Послушание). One id; owned on the heap by the
/// Testator; also kept in Temporality.
/// Ending a place (secede) is deferred for now — all kept places are active.
class Shepherding : public Place {
public:
	Shepherding(id::Obedience id, const Soul& testator, const Soul& executor);
	Shepherding(Shepherding&& other) noexcept;
	Shepherding& operator=(Shepherding&&) = delete;
	Shepherding(const Shepherding&) = delete;
	Shepherding& operator=(const Shepherding&) = delete;

	/// Shared place id (same value as the Obedience face).
	id::Obedience obedience_id() const noexcept { return id::Obedience{id().value()}; }

	const Soul& testator() const noexcept { return testator_; }
	const Soul& executor() const noexcept { return executor_; }

private:
	const Soul& testator_;
	const Soul& executor_;
};


} // namespace will::domain
