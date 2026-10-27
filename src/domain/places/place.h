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


/// Where one may stand: an abode, its rooms, its gates and its helm; and where a word is
/// fixed: an abode, a tie. A place reaches no dimension itself; Life recollects its words
/// for the rooms reflecting it, one recollection while a room onto it is beheld. Who may
/// stand in it, the witness says. From here one turns only where the place leads
/// him: anywhere within an abode, out of it only through its gates, and to another only from
/// one's own gates. A room asks the place it reflects whether it takes its host's words, and
/// has him do the deed they are made by: an abode takes a letter, a tie a behest. The roles
/// of a tie share it as a virtual base, given its id by the living Tie.
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
