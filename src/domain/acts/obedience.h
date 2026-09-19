#pragma once

#include "beings/place.h"
#include "identity/obedience.h"


namespace will::domain {


class Soul;


/// Obedience (Послушание) — Executor-facing interface of a shared Place:
/// ordered pair (testator, executor). Same place as Shepherding (Ведение);
/// one id, one living flag. Where a Deed (Дело) is kept; born from accepted
/// Supplication. Owned on the heap by the Executor; also kept in Temporality.
class Obedience : public Place {
public:
	Obedience(id::Obedience id, const Soul& testator, const Soul& executor, bool living = true);
	Obedience(Obedience&& other) noexcept;
	Obedience& operator=(Obedience&&) = delete;
	Obedience(const Obedience&) = delete;
	Obedience& operator=(const Obedience&) = delete;

	/// Strongly typed obedience id (same value as Place::id()).
	id::Obedience obedience_id() const noexcept { return id::Obedience{id().value()}; }

	const Soul& testator() const noexcept { return testator_; }
	const Soul& executor() const noexcept { return executor_; }
	bool living() const noexcept { return living_; }

	/// End this face of the place (secede). Throws if already ended.
	void end();

private:
	const Soul& testator_;
	const Soul& executor_;
	bool living_;
};


} // namespace will::domain
