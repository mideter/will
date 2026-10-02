#pragma once

#include "beings/immanents/abode.h"

#include <vector>


namespace will::domain {


class Letter;
class Witness;


/// Contemplation (Созерцание) — relation in Heaven: which Abode a soul observes.
/// The letters of an abode are seen through the contemplation of it.
class Contemplation {
public:
	Contemplation(const Witness& who, const Abode& abode);

	const Witness& who() const noexcept { return *who_; }
	const Abode& abode() const noexcept { return *abode_; }

	/// Letters of the contemplated abode, oldest first.
	std::vector<Letter> letters() const;

private:
	const Witness* who_;
	const Abode* abode_;
};


} // namespace will::domain
