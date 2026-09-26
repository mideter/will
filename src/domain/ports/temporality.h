#pragma once

#include "acts/dating.h"
#include "acts/embodiment.h"
#include "beings/immanents/obedience.h"
#include "acts/supplication.h"
#include "acts/tying.h"
#include "beings/deed.h"
#include "beings/immanents/soul.h"
#include "identity/deed.h"
#include "identity/letter.h"
#include "identity/soul.h"
#include "identity/tie.h"
#include "identity/vessel.h"
#include "values/device_token.h"
#include "values/soul_name.h"
#include "values/timestamp.h"
#include "values/saying.h"

#include <cstdint>
#include <utility>
#include <vector>


namespace will::domain {


class Novice;
class Testator;


/// Temporality (Временность) — what happens in time.
/// Earth speaks with this face (as Heaven speaks with Eternity).
class Temporality {
public:
	virtual ~Temporality() = default;

	/// Record that this soul dwells in a new vessel; returns material for World birth.
	virtual Embodiment embody(id::Soul soul, SoulName name, DeviceToken token) = 0;

	/// All embodiments kept in time (for Creation awaken).
	virtual std::vector<Embodiment> embodiments() const = 0;

	/// Fix a word at a moment in time.
	virtual void date(id::Letter id, Timestamp at) = 0;

	/// Datings for the given ids (skips undated).
	virtual std::vector<Dating> datings(const std::vector<id::Letter>& ids) const = 0;

	/// Offer obedience: suppliant asks addressee to become Завещатель.
	virtual Supplication supplicate(const Novice& suppliant, const Testator& addressee) = 0;

	/// Pending supplications addressed to this soul.
	virtual std::vector<Supplication> pending_supplications(id::Soul addressee) const = 0;

	/// Accept a pending; keeps the tying in time. Living Tie is born on the heap.
	/// Keyed by the living pair (suppliant, addressee).
	virtual Tying accept(const Supplication& ask) = 0;

	/// Reject a pending supplication (same living pair).
	virtual void reject(const Supplication& ask) = 0;

	/// Tying remembered in time. Throws if unknown.
	virtual Tying tying(id::Tie id) const = 0;

	/// All tyings kept in time.
	virtual std::vector<Tying> tyings() const = 0;

	/// Will a Saying in a shared place (testator must be that place's testator).
	virtual Deed will(const Obedience& obedience, const Soul& testator, const Saying& saying) = 0;

	/// Execute an open deed.
	virtual Deed execute(const Deed& deed) = 0;

	/// Deed by id. Throws if unknown.
	virtual Deed deed(id::Deed id) const = 0;

	/// Deeds kept in this shared place (any status).
	virtual std::vector<Deed> deeds(id::Tie tie) const = 0;
};


} // namespace will::domain
