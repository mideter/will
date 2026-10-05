#pragma once

#include "places/place.h"
#include "matter/deed.h"
#include "properties/birth.h"

#include <memory>
#include <vector>


namespace will::domain {


class Behest;
class Deed;
class Novice;
class Soul;
class Testator;


/// Obedience (Послушание) — Novice-facing interface of a shared Place.
/// Living heap object is Tie (Узы), owned by the Novice. Holds the
/// counterpart side (testator); accessors are completed by Tie. Ending a
/// place (secede) is deferred for now — all kept places are active.
class Obedience : public virtual Place {
public:
	~Obedience() override = default;

	virtual const Testator& testator() const = 0;
	virtual const Novice& novice() const = 0;

	/// Behests placed here, oldest first, shown to a side of this place.
	virtual std::vector<std::shared_ptr<const Behest>> behests(const Novice& asker) const = 0;

	/// Deeds placed here, oldest first, shown to a side of this place.
	virtual std::vector<std::shared_ptr<const Deed>> deeds(const Novice& asker) const = 0;

	/// A deed the novice brings forth here, born living from the matter the
	/// dimensions returned when they kept it.
	virtual std::shared_ptr<const Deed> inscribe(Birth<Novice>, matter::Deed kept) const = 0;

protected:
	explicit Obedience(const Soul& testator);

	const Testator& testator_;
};


} // namespace will::domain
