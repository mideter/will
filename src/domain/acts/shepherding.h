#pragma once

#include "beings/place.h"


namespace will::domain {


class Novice;
class Testator;


/// Shepherding (Ведение) — Testator-facing interface of the shared Place.
/// Living heap object is Tie (Узы), owned by the Novice as Obedience.
/// Holds the counterpart side (novice); accessors are completed by Tie.
/// Ending a place (secede) is deferred for now — all kept places are active.
class Shepherding : public virtual Place {
public:
	~Shepherding() override = default;

	Shepherding(const Shepherding&) = delete;
	Shepherding& operator=(const Shepherding&) = delete;

	virtual const Testator& testator() const = 0;
	virtual const Novice& novice() const = 0;

protected:
	explicit Shepherding(const Novice& novice);
	Shepherding& operator=(Shepherding&&) = delete;

	const Novice& novice_;
};


} // namespace will::domain
