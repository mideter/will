#pragma once

#include "matter/dweller.h"
#include "matter/room.h"
#include "places/place.h"
#include "properties/birth.h"
#include "properties/immanent.h"

#include <atomic>
#include <string>


namespace will::domain {


class Abode;
class Heaven;


/// Room (Комната) — a window in an Abode onto what it reflects, with no words of its
/// own: Слово onto the abode, Ведение and Послушание onto a tie.
class Room : public Place {
public:
	Room(Birth<Abode> birth, const Place& reflects, matter::Room kept);

	const Abode& abode() const noexcept { return abode_; }
	const Place& reflects() const noexcept { return reflects_; }
	matter::Room::Aspect aspect() const noexcept { return aspect_; }
	matter::Room::Part part() const noexcept { return part_.load(); }

	virtual std::string name() const;

	void arrange(Birth<Abode> birth, const matter::Room& kept) const;

	/// The host, and the dwellers whose kind enters its part.
	bool dwells(const Man& man) const override;

	const Place& source() const override;

	/// One writes in a room of one's own abode that shows words.
	std::exception_ptr refuses_word_of(const Man& writer) const override;

	bool shows(const Man& who, const Word& word) const override;

private:
	const Abode& abode_;
	const Place& reflects_;
	const matter::Room::Aspect aspect_;
	mutable std::atomic<matter::Room::Part> part_;
};


/// Gates (Врата) — the threshold of an Abode, through which one comes in, born or to be
/// born: those who keep them let in an acquaintance of the host and bear a child of
/// his, whoever does it.
class Gates final : public Room, private Immanent<Heaven> {
public:
	Gates(Birth<Abode> birth, matter::Room kept);

	std::string name() const override;

	/// Anyone may stand at the gates.
	bool dwells(const Man& man) const override;

	/// The host, or a dweller whose kind enters the part they stand in.
	bool keeps(const Man& man) const;

	/// Open while one who keeps them stands in them.
	bool open() const;

	void admit(Birth<Man> keeper, const Man& man, const matter::Dweller& kept) const;

	const Place& source() const override;
	bool shows(const Man& who, const Word& word) const override;
};


/// UpperRoom (Горница) — the room of an Abode's dwellers: here alone the host regards
/// them anew and arranges his rooms.
class UpperRoom final : public Room {
public:
	UpperRoom(Birth<Abode> birth, matter::Room kept);

	std::string name() const override;

	const Place& source() const override;
	bool shows(const Man& who, const Word& word) const override;
};


} // namespace will::domain
