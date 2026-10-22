#pragma once

#include "places/place.h"
#include "properties/immanent.h"
#include "properties/birth.h"

#include <memory>
#include <vector>


namespace will::domain {


class Cell;
class Gates;
class Heaven;
class Recollection;
class Helm;
class Man;
class Word;


/// The gaze of a spirit, the eye of a soul, one at a time: the words of a place
/// live while it is beheld.
class Contemplation : public Immanent<Heaven> {
public:
	const Man& who() const noexcept { return who_; }
	const Place& place() const noexcept { return place_; }

	/// Those the place shows to who looks, oldest first.
	std::vector<std::shared_ptr<const Word>> words() const;

	/// The room one stands in, of the kind one would act in.
	const Cell& cell() const;
	const Helm& helm() const;
	const Gates& gates() const;

	/// Whether what one writes here is taken.
	bool writable() const;

	Contemplation(Birth<Heaven>, const Man& who, const Place& place);

private:
	const Man& who_;
	const Place& place_;
	std::shared_ptr<const Recollection> recollection_;
};


} // namespace will::domain
