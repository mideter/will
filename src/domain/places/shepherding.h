#pragma once

#include "places/place.h"
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


/// Shepherding (Ведение) — a tie as its testator sees it.
class Shepherding : public virtual Place {
public:
	~Shepherding() override = default;

	virtual const Testator& testator() const = 0;
	virtual const Novice& novice() const = 0;

	/// Oldest first.
	virtual std::vector<std::shared_ptr<const Behest>> behests(const Novice& asker) const = 0;

	virtual std::shared_ptr<const Behest> inscribe(Birth<Testator>, matter::Behest kept) const = 0;

	/// Oldest first.
	virtual std::vector<std::shared_ptr<const Deed>> deeds(const Novice& asker) const = 0;

protected:
	explicit Shepherding(const Soul& novice);

	const Novice& novice_;
};


} // namespace will::domain
