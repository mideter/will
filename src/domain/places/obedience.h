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
class Man;


/// A tie as its novice sees it.
class Obedience : public virtual Place {
public:
	~Obedience() override = default;

	virtual const Man& testator() const = 0;
	virtual const Man& novice() const = 0;

	/// Oldest first.
	virtual std::vector<std::shared_ptr<const Behest>> behests(const Novice& asker) const = 0;

	/// Oldest first.
	virtual std::vector<std::shared_ptr<const Deed>> deeds(const Novice& asker) const = 0;

	virtual std::shared_ptr<const Deed> inscribe(Birth<Novice>, matter::Deed kept) const = 0;

protected:
	explicit Obedience(const Man& testator);

	const Man& testator_;
};


} // namespace will::domain
