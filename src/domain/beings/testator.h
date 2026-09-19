#pragma once

#include "acts/embodiment.h"
#include "acts/obedience.h"
#include "acts/shepherding.h"
#include "acts/supplication.h"
#include "beings/executor.h"
#include "beings/deed.h"
#include "values/word.h"

#include <functional>
#include <memory>
#include <vector>


namespace will::domain {


class World;


/// Testator (Завещатель, Тренер) — Executor who may pass received will on as his own.
/// Owns Shepherding (Ведение) faces and incoming pending Supplications on the heap;
/// Temporality keeps the same in time. Only World may birth a Testator onto the heap.
class Testator : public Executor {
public:
	/// Accept a pending incoming supplication; entrusts the act to birth both faces.
	/// Returns the Executor-owned Obedience.
	const Obedience& accept(const Supplication& supplication) const;

	/// Refuse a pending incoming; entrusts the act to dismiss itself.
	void refuse(const Supplication& supplication) const;

	/// Bequeath a word through the Shepherding face of a shared place.
	Deed will(const Shepherding& shepherding, const Word& word) const;

	/// Owned Shepherding face by place id. Throws if unknown.
	const Shepherding& shepherding(id::Obedience id) const;

	/// Owned incoming pending from this executor (suppliant). Throws if unknown.
	const Supplication& supplication(const Executor& executor) const;

	/// Incoming pending supplications kept on this testator.
	std::vector<std::reference_wrapper<const Supplication>> supplications() const;

private:
	friend class World;
	friend class Supplication;

	explicit Testator(Embodiment embodiment);

	/// Keep a Shepherding face on this testator (entrusted Supplication / awaken).
	const Shepherding& keep(Shepherding place) const;

	/// Receive an incoming pending (entrusted Supplication / awaken).
	const Supplication& receive(Supplication supplication) const;

	/// Drop an incoming after consent or dismiss (keyed by executor).
	void drop_supplication(const Executor& executor) const;

	mutable std::vector<std::unique_ptr<Shepherding>> shepherdings_;
	mutable std::vector<std::unique_ptr<Supplication>> incoming_;
};


} // namespace will::domain
