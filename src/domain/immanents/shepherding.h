#pragma once

#include "immanents/place.h"
#include "matter/behest.h"
#include "properties/birth.h"

#include <memory>
#include <vector>


namespace will::domain {


class Behest;
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

	virtual const Testator& testator() const = 0;
	virtual const Novice& novice() const = 0;

	/// Behests placed here, oldest first, shown to a side of this place.
	virtual std::vector<std::shared_ptr<const Behest>> behests(const Novice& asker) const = 0;

	/// A behest the testator wills here, born living from the matter the
	/// dimensions returned when they kept it.
	virtual std::shared_ptr<const Behest> inscribe(Birth<Testator>, matter::Behest kept) const = 0;

	/// Deeds placed here, oldest first, shown to a side of this place.
	virtual std::vector<std::shared_ptr<const Deed>> deeds(const Novice& asker) const = 0;

protected:
	explicit Shepherding(const Soul& novice);

	const Novice& novice_;
};


} // namespace will::domain
