#pragma once

#include "matter/room.h"
#include "places/place.h"
#include "properties/birth.h"

#include <string>


namespace will::domain {


class Abode;


/// Room (Комната) — a window in an Abode onto the place it reflects; it has no
/// words of its own. Its name is given by what it reflects: the Cell (Келья)
/// reflects the Abode itself, a tie room reflects a Tie — Ведение for its
/// testator, Послушание for its novice. Born and held by its Abode.
class Room : public Place {
public:
	Room(Birth<Abode> birth, const Place& reflects, matter::Room kept);

	const Abode& abode() const noexcept { return abode_; }
	const Place& reflects() const noexcept { return reflects_; }
	matter::Room::Part part() const noexcept { return part_; }

	std::string name() const;

	/// The host of its abode dwells here.
	bool dwells(const Man& man) const override;

private:
	const Abode& abode_;
	const Place& reflects_;
	matter::Room::Part part_;
};


} // namespace will::domain
