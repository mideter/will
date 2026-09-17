#pragma once

#include "entities/place.h"
#include "identity/obedience.h"
#include "identity/soul.h"


namespace will::domain {


/// Obedience (Послушание) — ordered pair (testator, executor) as a Place.
/// Where a Testament may later be fixed; born from accepted Supplication.
class Obedience : public Place {
public:
	Obedience(id::Obedience id, id::Soul testator, id::Soul executor, bool living = true);
	Obedience(Obedience&& other) noexcept;
	Obedience& operator=(Obedience&& other) noexcept;
	Obedience(const Obedience&) = delete;
	Obedience& operator=(const Obedience&) = delete;

	/// Strongly typed obedience id (same value as Place::id()).
	id::Obedience obedience_id() const noexcept { return id::Obedience{id().value()}; }

	id::Soul testator() const noexcept { return testator_; }
	id::Soul executor() const noexcept { return executor_; }
	bool living() const noexcept { return living_; }

private:
	id::Soul testator_;
	id::Soul executor_;
	bool living_;
};


} // namespace will::domain
