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


/// Obedience (Послушание) — a tie as its novice sees it.
class Obedience : public virtual Place {
public:
	~Obedience() override = default;

	virtual const Testator& testator() const = 0;
	virtual const Novice& novice() const = 0;

	/// Oldest first.
	virtual std::vector<std::shared_ptr<const Behest>> behests(const Novice& asker) const = 0;

	/// Oldest first.
	virtual std::vector<std::shared_ptr<const Deed>> deeds(const Novice& asker) const = 0;

	virtual std::shared_ptr<const Deed> inscribe(Birth<Novice>, matter::Deed kept) const = 0;

protected:
	explicit Obedience(const Soul& testator);

	const Testator& testator_;
};


} // namespace will::domain
