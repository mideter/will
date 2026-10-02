#pragma once

#include "immanents/place.h"

#include <vector>


namespace will::domain {


class Deed;
class Novice;
class Soul;
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

	/// Deeds placed here, oldest first, shown to a side of this place.
	virtual std::vector<Deed> deeds(const Novice& asker) const = 0;

protected:
	explicit Shepherding(const Soul& novice);
	Shepherding& operator=(Shepherding&&) = delete;

	const Novice& novice_;
};


} // namespace will::domain
