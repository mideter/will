#pragma once

#include "matter/fatherhood.h"
#include "matter/man.h"
#include "places/abode.h"
#include "men/soul.h"
#include "men/vessel.h"

#include <atomic>
#include <memory>


namespace will::domain {


class UpperRoom;
class World;


/// Man (Человек) — a soul dwelling in a body, with his own abode.
class Man : public Soul, public Vessel {
public:
	~Man() override;

	Abode& abode() const noexcept { return *abode_; }

	/// He becomes an acquaintance of the host of the gates, whoever lets him in.
	void admit(const Man& man) const;

	/// In the upper room one stands in.
	void regard(const Man& dweller, matter::Dweller::Kind kind) const;
	void regard(Birth<UpperRoom> upper_room, const Man& dweller, matter::Dweller::Kind kind) const;

	/// In the upper room one stands in.
	void arrange(const Room& room, matter::Room::Part part) const;
	void arrange(Birth<UpperRoom> upper_room, const Room& room, matter::Room::Part part) const;

	const Man* father(matter::Fatherhood::Line line) const noexcept;

	void descend(Birth<World> birth, const Man& father, const matter::Fatherhood& kept) const;

	bool operator==(const Man& other) const = default;

protected:
	explicit Man(matter::Man kept);

private:
	std::unique_ptr<Abode> abode_;
	mutable std::atomic<const Man*> father_by_flesh_ = nullptr;
	mutable std::atomic<const Man*> father_by_spirit_ = nullptr;
};


} // namespace will::domain
