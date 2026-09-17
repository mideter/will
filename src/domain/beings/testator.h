#pragma once

#include "acts/embodiment.h"
#include "acts/obedience.h"
#include "acts/supplication.h"
#include "beings/executor.h"
#include "beings/testament.h"
#include "values/word.h"


namespace will::domain {


class World;


/// Testator (Завещатель) — Executor who may pass received will on as his own.
/// All living modes are present from the start; situations disclose them.
/// Only World may birth a Testator onto the heap.
class Testator : public Executor {
public:
	/// Accept a pending supplication addressed to this soul; births Obedience.
	Obedience accept(const Supplication& supplication) const;

	/// Refuse a pending supplication addressed to this soul.
	void refuse(const Supplication& supplication) const;

	/// Bequeath a word in a living obedience where this soul is testator.
	Testament will(const Obedience& obedience, const Word& word) const;

private:
	friend class World;

	explicit Testator(Embodiment embodiment);
};


} // namespace will::domain
