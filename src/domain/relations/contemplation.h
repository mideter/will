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


/// Contemplation (Созерцание) — the gaze of a soul, one at a time: the words of a place
/// live while it is beheld.
class Contemplation : public Immanent<Heaven> {
public:
	const Witness& who() const noexcept { return who_; }
	const Place& place() const noexcept { return place_; }

	/// Those the place shows to who looks, oldest first.
	std::vector<std::shared_ptr<const Word>> words() const;

	Contemplation(Birth<Heaven>, const Witness& who, const Place& place);

private:
	const Witness& who_;
	const Place& place_;
	std::shared_ptr<const Recollection> recollection_;
};


} // namespace will::domain
