#pragma once

#include "acts/embodiment.h"
#include "acts/obedience.h"
#include "acts/shepherding.h"
#include "acts/supplication.h"
#include "beings/executor.h"
#include "beings/deed.h"
#include "identity/supplication.h"
#include "values/word.h"

#include <functional>
#include <memory>
#include <vector>


namespace will::domain {


class World;
class Executor;


/// Testator (Завещатель, Тренер) — Executor who may pass received will on as his own.
/// Owns Shepherding (Ведение) faces and incoming pending Supplications on the heap;
/// Temporality keeps the same in time. Only World may birth a Testator onto the heap.
class Testator : public Executor {
public:
	/// Accept a pending incoming supplication; births the shared place on both heap
	/// faces and in Temporality. Returns the Executor-owned Obedience.
	const Obedience& accept(const Supplication& supplication) const;

	/// Refuse a pending incoming supplication.
	void refuse(const Supplication& supplication) const;

	/// Bequeath a word through the Shepherding face of a shared place.
	Deed will(const Shepherding& shepherding, const Word& word) const;

	/// Owned Shepherding face by place id. Throws if unknown.
	const Shepherding& shepherding(id::Obedience id) const;

	/// Owned incoming pending Supplication by id. Throws if unknown.
	const Supplication& supplication(id::Supplication id) const;

	/// Incoming pending supplications kept on this testator.
	std::vector<std::reference_wrapper<const Supplication>> pending_supplications() const;

private:
	friend class World;
	friend class Executor;

	explicit Testator(Embodiment embodiment);

	/// Keep a Shepherding face on this testator (accept / awaken).
	const Shepherding& keep(Shepherding place) const;

	/// Receive an incoming pending supplication (supplicate / awaken).
	const Supplication& receive(Supplication supplication) const;

	/// Drop an incoming supplication after accept or refuse.
	void drop_supplication(id::Supplication id) const;

	mutable std::vector<std::unique_ptr<Shepherding>> shepherdings_;
	mutable std::vector<std::unique_ptr<Supplication>> incoming_;
};


} // namespace will::domain
