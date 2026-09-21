#pragma once

#include "acts/embodiment.h"
#include "acts/obedience.h"
#include "acts/supplication.h"
#include "beings/abode.h"
#include "beings/letter.h"
#include "beings/soul.h"
#include "beings/deed.h"
#include "identity/abode.h"
#include "identity/deed.h"
#include "identity/obedience.h"
#include "identity/place.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "ports/eternity.h"
#include "values/abode_name.h"
#include "values/device_token.h"
#include "values/soul_name.h"
#include "values/word.h"

#include <cstdint>
#include <utility>
#include <vector>


namespace will::domain {


class Novice;
class Testator;


/// Temporality (Временность) — mutable Eternity; what happens in time.
/// Earth speaks with this face (as Heaven speaks with Eternity).
class Temporality : public Eternity {
public:
	static constexpr std::uint32_t MaxLetterLimit = 1000;

	~Temporality() override = default;

	/// Record that this soul dwells in a new vessel; returns material for World birth.
	virtual Embodiment embody(id::Soul soul, DeviceToken token) = 0;

	/// All embodiments kept in time (for Creation awaken).
	virtual std::vector<Embodiment> embodiments() const = 0;

	/// Abodes kept in time (id + name).
	virtual std::vector<Abode> abodes() = 0;

	/// Keep an abode in time (idempotent by id).
	virtual void keep(id::Abode id, AbodeName name) = 0;

	/// Record that a soul dwells in an abode (idempotent).
	virtual void join_abode(id::Abode abode, id::Soul soul) = 0;

	/// Fix a word in time: take the present from Time and keep the letter.
	virtual void fix(id::Place place, id::Soul author, const Word& word) const = 0;

	/// Letters kept in this place, bounded by limit (history).
	virtual std::vector<Letter> letters(id::Place place, std::uint32_t limit) const = 0;

	/// Offer obedience: suppliant asks addressee to become Завещатель.
	virtual Supplication supplicate(const Novice& suppliant, const Testator& addressee) = 0;

	/// Pending supplications addressed to this soul.
	virtual std::vector<Supplication> pending_supplications(id::Soul addressee) const = 0;

	/// Accept a pending supplication; keeps the shared Obedience/Shepherding place in time.
	/// Keyed by the living pair (suppliant, addressee).
	virtual Obedience accept(const Supplication& ask) = 0;

	/// Reject a pending supplication (same living pair).
	virtual void reject(const Supplication& ask) = 0;

	/// Shared place by id (Obedience face). Throws if unknown.
	virtual Obedience obedience(id::Obedience id) const = 0;

	/// All shared places kept in time (Obedience faces).
	virtual std::vector<Obedience> obediences() const = 0;

	/// Bequeath a word in a shared place (testator must be that place's testator).
	virtual Deed bequeath(const Obedience& obedience, const Soul& testator, const Word& word) = 0;

	/// Execute an open deed.
	virtual Deed execute(const Deed& deed) = 0;

	/// Deed by id. Throws if unknown.
	virtual Deed deed(id::Deed id) const = 0;

	/// Deeds kept in this shared place (any status).
	virtual std::vector<Deed> deeds(id::Obedience obedience) const = 0;
};


} // namespace will::domain
