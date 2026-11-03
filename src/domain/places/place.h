#pragma once

#include "identity/place.h"
#include "properties/birth.h"
#include "properties/immanent.h"

#include <memory>
#include <optional>
#include <vector>


namespace will::domain {


class OuterAbode;
class Room;
class Man;
class Space;
class Recollection;
class Saying;
class Word;


class Place : public Immanent<Space> {
public:
	virtual ~Place() = default;

	static const Place& of(id::Place id);

	id::Place id() const noexcept { return id_; }

	virtual bool leads_to(const Man& who, const Place& next) const;

	virtual bool within(const OuterAbode& abode) const;

	virtual bool takes_words_of(const Man& host) const;

	virtual std::shared_ptr<const Word> receive(Birth<Room> room, const Saying& saying) const;

	std::shared_ptr<const Recollection> recollection(Birth<Room> room) const;

protected:
	std::shared_ptr<const Recollection> recollection() const;

	void enter(const Recollection& recollection, std::shared_ptr<const Word> word) const;

	Place();
	explicit Place(id::Place id) noexcept;

private:
	id::Place id_;
};


} // namespace will::domain
