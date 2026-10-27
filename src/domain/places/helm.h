#pragma once

#include "matter/dweller.h"
#include "matter/room.h"
#include "places/place.h"
#include "properties/birth.h"


namespace will::domain {


class Abode;
class Room;
class Contemplation;
class Man;
class OuterAbode;
class Shepherding;
class Supplication;


/// The host's own place, the inner edge of his abode, whence he steers his abode; no one
/// else enters. Here alone he answers those who ask him and asks himself, regards his
/// dwellers anew and moves his rooms outward and inward.
class Helm final : public Place {
public:
	Helm(Birth<Abode> birth, id::Place id);

	const Abode& abode() const noexcept { return abode_; }

	bool leads_to(const Man& who, const Place& next) const override;

	bool within(const OuterAbode& abode) const override;

	void regard(const Contemplation& gaze, const Man& dweller, matter::Dweller::Kind kind) const;

	void arrange(const Contemplation& gaze, const Room& room, matter::Room::Part to) const;

	void supplicate(const Contemplation& gaze, const Man& addressee) const;

	const Shepherding& accept(const Contemplation& gaze, const Supplication& ask) const;
	void reject(const Contemplation& gaze, const Supplication& ask) const;

private:
	const Abode& abode_;
};


} // namespace will::domain
