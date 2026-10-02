#pragma once

#include "immanents/abode.h"
#include "properties/immanent.h"

#include <vector>


namespace will::domain {


class Heaven;
class Letter;
class Witness;


/// Contemplation (Созерцание) — the gaze of a soul upon an Abode; immanent to Heaven.
/// Heaven keeps it, one per soul, from the moment the soul's man wakes or turns
/// to an abode until he turns elsewhere or falls asleep.
/// The letters of an abode are seen through the contemplation of it.
class Contemplation : public Immanent<Heaven> {
public:
	const Witness& who() const noexcept { return who_; }
	const Abode& abode() const noexcept { return abode_; }

	/// Letters of the contemplated abode, oldest first.
	std::vector<Letter> letters() const;

private:
	friend class Heaven;

	Contemplation(const Witness& who, const Abode& abode);

	const Witness& who_;
	const Abode& abode_;
};


} // namespace will::domain
