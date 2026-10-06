#pragma once

#include "matter/fatherhood.h"
#include "matter/man.h"
#include "places/abode.h"
#include "men/soul.h"
#include "men/vessel.h"

#include <atomic>
#include <memory>


namespace will::domain {


class World;


/// Man (Человек) — soul dwelling in a vessel. One object is both Soul and Vessel.
/// Owns his personal Abode. Only heirs construct Man; World births the living
/// heap object as Testator. Construction presents soul and vessel to Heaven and Earth.
/// One soul — one living man in the World; living identity is Soul::id().
class Man : public Soul, public Vessel {
public:
	~Man() override;

	Abode& abode() const noexcept { return *abode_; }

	/// Let a man in as an acquaintance: one stands in one's gates and he stands
	/// at them. Throws if he already dwells here or either is not at the gates.
	void admit(const Man& man) const;

	/// Regard a dweller of one's abode as of this kind, standing in one's
	/// upper room. Throws if he does not dwell there or one is elsewhere.
	void regard(const Man& dweller, matter::Dweller::Kind kind) const;

	/// Set a room of one's abode in this part of it. Throws if the room is of
	/// another abode.
	void arrange(const Room& room, matter::Room::Part part) const;

	/// One's father along the line; none if one has none.
	const Man* father(matter::Fatherhood::Line line) const noexcept;

	/// One descends from this father along the line, as the fatherhood matter
	/// says: the World tells it at birth, at choosing and on awakening.
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
