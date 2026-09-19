#pragma once

#include "acts/embodiment.h"
#include "acts/obedience.h"
#include "acts/shepherding.h"
#include "acts/supplication.h"
#include "beings/executor.h"
#include "beings/deed.h"
#include "values/word.h"

#include <memory>
#include <vector>


namespace will::domain {


class World;


/// Testator (Завещатель, Тренер) — Executor who may pass received will on as his own.
/// Owns Shepherding (Ведение) faces of the shared place on the heap; same Place as
/// the Executor's Obedience. All living modes are present from the start; situations
/// disclose them. Only World may birth a Testator onto the heap.
class Testator : public Executor {
public:
	/// Accept a pending supplication addressed to this soul; births the shared
	/// place on both heap faces and in Temporality. Returns the Executor-owned Obedience.
	const Obedience& accept(const Supplication& supplication) const;

	/// Refuse a pending supplication addressed to this soul.
	void refuse(const Supplication& supplication) const;

	/// Bequeath a word through the Shepherding face of a living shared place.
	Deed will(const Shepherding& shepherding, const Word& word) const;

	/// Leave a living shared place (Shepherding face).
	void secede(const Shepherding& shepherding) const;

	/// Owned Shepherding face by place id. Throws if unknown.
	const Shepherding& shepherding(id::Obedience id) const;

private:
	friend class World;
	friend class Executor;

	explicit Testator(Embodiment embodiment);

	/// Keep a Shepherding face on this testator (accept / awaken).
	const Shepherding& keep(Shepherding place) const;

	/// End the owned Shepherding face. Throws if unknown or already ended.
	void end_shepherding(id::Obedience id) const;

	mutable std::vector<std::unique_ptr<Shepherding>> shepherdings_;
};


} // namespace will::domain
