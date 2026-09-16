#pragma once

#include "entities/executor.h"
#include "entities/man.h"
#include "entities/obedience.h"
#include "entities/supplication.h"


namespace will::domain {


/// Testator (Завещатель) — Executor who may pass received will on as his own.
/// All living modes are present from the start; situations disclose them.
/// World::accept always places a Testator on the heap.
class Testator : public Executor {
public:
	explicit Testator(Man&& man);

	/// Accept a pending supplication addressed to this soul; births Obedience.
	Obedience accept(const Supplication& supplication) const;

	/// Refuse a pending supplication addressed to this soul.
	void refuse(const Supplication& supplication) const;
};


} // namespace will::domain
