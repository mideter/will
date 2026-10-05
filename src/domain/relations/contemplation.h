#pragma once

#include "places/place.h"
#include "properties/immanent.h"
#include "properties/birth.h"

#include <memory>
#include <vector>


namespace will::domain {


class Heaven;
class Recollection;
class Witness;
class Word;


/// Contemplation (Созерцание) — the gaze of a soul upon a Place he dwells in
/// (his Abode, or a Tie he is a side of); immanent to Heaven.
/// Heaven keeps it, one per soul, from the moment the soul's man wakes or turns
/// to a place until he turns elsewhere or falls asleep.
/// The words of a place live while they are beheld: each gaze holds the
/// Recollection of its place, all gazes upon the place share the same one.
class Contemplation : public Immanent<Heaven> {
public:
	const Witness& who() const noexcept { return who_; }
	const Place& place() const noexcept { return place_; }

	/// Living words of the contemplated place that it shows to who looks, oldest first.
	std::vector<std::shared_ptr<const Word>> words() const;

	Contemplation(Birth<Heaven>, const Witness& who, const Place& place);

private:
	const Witness& who_;
	const Place& place_;
	std::shared_ptr<const Recollection> recollection_;
};


} // namespace will::domain
