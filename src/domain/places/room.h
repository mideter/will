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


/// Room (Комната) — a window in an Abode onto what it reflects; it has no words
/// of its own. Its name is given by what it reflects: the Cell (Келья) the words
/// of the Abode itself, a tie room the words of a Tie — Ведение for its testator,
/// Послушание for its novice; the Gates its threshold, the upper room (Горница) its
/// dwellers. Born and held by its Abode.
class Room : public Place {
public:
	Room(Birth<Abode> birth, const Place& reflects, matter::Room kept);

	const Abode& abode() const noexcept { return abode_; }
	const Place& reflects() const noexcept { return reflects_; }
	matter::Room::Aspect aspect() const noexcept { return aspect_; }
	matter::Room::Part part() const noexcept { return part_.load(); }

	virtual std::string name() const;

	/// The room stands now where its matter says: its host arranged it.
	void arrange(Birth<Abode> birth, const matter::Room& kept) const;

	/// The host of its abode dwells here, and the dwellers whose kind enters its part.
	bool dwells(const Man& man) const override;

	/// A room shows the words of the place it reflects.
	const Place& source() const override;

	/// It shows them to whoever dwells here now.
	bool shows(const Man& who, const Word& word) const override;

private:
	const Abode& abode_;
	const Place& reflects_;
	const matter::Room::Aspect aspect_;
	mutable std::atomic<matter::Room::Part> part_;
};


/// Gates (Врата) — the room reflecting the threshold of an Abode: one comes in
/// through it born or to be born. Anyone who would come into another's Abode
/// enters its Gates by the host's name and stands there until he leaves. Those
/// who keep the Gates — the host, and the dwellers whose kind enters the part
/// they stand in — open them by standing there: they see who waits and may let
/// him in, an acquaintance of the host, whoever lets him in; and they see the
/// unborn and may bear one, a child of the host, whoever bears him. The Gates
/// show no words.
class Gates final : public Room, private Immanent<Heaven> {
public:
	Gates(Birth<Abode> birth, matter::Room kept);

	std::string name() const override;

	/// Anyone may stand at the gates.
	bool dwells(const Man& man) const override;

	/// Whether this man keeps the gates: the host, or a dweller whose kind
	/// enters the part they stand in.
	bool keeps(const Man& man) const;

	/// Open while one who keeps them stands in them.
	bool open() const;

	/// A keeper standing here lets in one who stands here and does not dwell in
	/// the abode, as the dweller matter says. Throws otherwise.
	void admit(Birth<Man> keeper, const Man& man, const matter::Dweller& kept) const;

	const Place& source() const override;
	bool shows(const Man& who, const Word& word) const override;
};


/// UpperRoom (Горница) — the room reflecting the dwellers of an Abode: those
/// who enter see them, and here alone the host regards them anew. It stands in a
/// part like any room. It shows no words.
class UpperRoom final : public Room {
public:
	UpperRoom(Birth<Abode> birth, matter::Room kept);

	std::string name() const override;

	const Place& source() const override;
	bool shows(const Man& who, const Word& word) const override;
};


} // namespace will::domain
