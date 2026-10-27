#pragma once

#include "matter/dweller.h"
#include "places/place.h"
#include "properties/birth.h"



namespace will::domain {


class Abode;
class Room;
class Contemplation;
class Inner;
class Man;
class Part;
class Shepherding;
class Supplication;


/// The host's own place, the inner edge of the inner part, whence he steers his abode; no one
/// else enters. Here alone he answers those who ask him and asks himself, regards his
/// dwellers anew and moves his rooms from part to part.
class Helm final : public Place {
public:
	Helm(Birth<Inner> birth, id::Place id);

	const Abode& abode() const noexcept;
	const Inner& part() const noexcept { return inner_; }

	bool dwells(const Man& man) const override;

	bool leads_to(const Man& who, const Place& next) const override;

	bool within(const Abode& abode) const override;

	void regard(const Contemplation& gaze, const Man& dweller, matter::Dweller::Kind kind) const;

	void arrange(const Contemplation& gaze, const Room& room, const Part& part) const;

	void supplicate(const Contemplation& gaze, const Man& addressee) const;

	const Shepherding& accept(const Contemplation& gaze, const Supplication& ask) const;
	void reject(const Contemplation& gaze, const Supplication& ask) const;

private:
	const Inner& inner_;
};


} // namespace will::domain
