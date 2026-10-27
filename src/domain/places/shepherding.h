#pragma once

#include "places/place.h"
#include "matter/behest.h"
#include "properties/birth.h"

#include <memory>
#include <vector>


namespace will::domain {


class Behest;
class Contemplation;
class Deed;
class Novice;
class Man;
class Testator;


/// A tie as its testator sees it; its behests and deeds come oldest first.
class Shepherding : public virtual Place {
public:
	~Shepherding() override = default;

	virtual const Man& testator() const = 0;
	virtual const Man& novice() const = 0;

	virtual std::vector<std::shared_ptr<const Behest>> behests(const Contemplation& gaze) const = 0;
	virtual std::vector<std::shared_ptr<const Behest>> behests(Birth<Testator> testator) const = 0;

	virtual std::shared_ptr<const Behest> inscribe(Birth<Testator>, matter::Behest kept) const = 0;

	virtual std::vector<std::shared_ptr<const Deed>> deeds(const Contemplation& gaze) const = 0;

protected:
	explicit Shepherding(const Man& novice);

	const Man& novice_;
};


} // namespace will::domain
