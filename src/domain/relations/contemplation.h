#pragma once

#include "places/place.h"
#include "properties/immanent.h"
#include "properties/birth.h"

#include <memory>
#include <vector>


namespace will::domain {


class Room;
class Gates;
class Heaven;
class Recollection;
class Helm;
class Man;
class Word;


class Contemplation : public Immanent<Heaven> {
public:
	virtual ~Contemplation() = default;

	const Man& who() const noexcept { return who_; }
	const Place& place() const noexcept { return place_; }

	const Room& room() const;
	const Helm& helm() const;
	const Gates& gates() const;

	bool writable() const;

	Contemplation(Birth<Heaven>, const Man& who, const Place& place);

private:
	const Man& who_;
	const Place& place_;
	std::shared_ptr<const Recollection> recollection_;
};


} // namespace will::domain
