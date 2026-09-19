#pragma once

#include "acts/embodiment.h"
#include "acts/obedience.h"
#include "acts/shepherding.h"
#include "acts/supplication.h"
#include "beings/novice.h"
#include "beings/deed.h"
#include "values/word.h"

#include <functional>
#include <memory>
#include <vector>


namespace will::domain {


class World;


/// Testator (Завещатель, Тренер) — Novice who may pass received will on as his own.
/// Owns Shepherding (Ведение) faces and incoming pending Supplications on the heap;
/// Temporality keeps the same in time. Only World may birth a Testator onto the heap.
class Testator : public Novice {
public:
	/// Accept a pending incoming; the act signs and births both faces.
	void accept(const Supplication& ask) const;

	/// Refuse a pending incoming; the act rejects itself.
	void refuse(const Supplication& ask) const;

	/// Bequeath a word through the Shepherding face of a shared place.
	Deed will(const Shepherding& shepherding, const Word& word) const;

	/// Owned Shepherding face by place id. Throws if unknown.
	const Shepherding& shepherding(id::Obedience id) const;

	/// Owned incoming pending from this novice (suppliant). Throws if unknown.
	const Supplication& supplication(const Novice& novice) const;

	/// Incoming pending supplications kept on this testator.
	std::vector<std::reference_wrapper<const Supplication>> supplications() const;

private:
	friend class World;
	friend class Supplication;

	explicit Testator(Embodiment embodiment);

	/// Keep a Shepherding face on this testator (signed Supplication / awaken).
	const Shepherding& keep(Shepherding place) const;

	/// Receive an incoming pending (signed Supplication / awaken).
	const Supplication& receive(Supplication supplication) const;

	/// Drop an incoming after sign or reject (keyed by novice).
	void drop_supplication(const Novice& novice) const;

	mutable std::vector<std::unique_ptr<Shepherding>> shepherdings_;
	mutable std::vector<std::unique_ptr<Supplication>> incoming_;
};


} // namespace will::domain
