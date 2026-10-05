#pragma once

#include "matter/man.h"
#include "places/abode.h"
#include "men/soul.h"
#include "men/vessel.h"

#include <memory>


namespace will::domain {


/// Man (Человек) — soul dwelling in a vessel. One object is both Soul and Vessel.
/// Owns his personal Abode. Only heirs construct Man; World births the living
/// heap object as Testator. Construction presents soul and vessel to Heaven and Earth.
/// One soul — one living man in the World; living identity is Soul::id().
class Man : public Soul, public Vessel {
public:
	~Man() override;

	Abode& abode() const noexcept { return *abode_; }

	/// Admit a man into one's abode as an acquaintance. Throws if he already
	/// dwells there.
	void admit(const Man& man) const;

	/// Regard a dweller of one's abode as of this kind. Throws if he does not
	/// dwell there.
	void regard(const Man& dweller, matter::Dweller::Kind kind) const;

	bool operator==(const Man& other) const = default;

protected:
	explicit Man(matter::Man kept);

private:
	std::unique_ptr<Abode> abode_;
};


} // namespace will::domain
