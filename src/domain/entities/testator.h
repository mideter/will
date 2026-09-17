#pragma once

#include "entities/executor.h"
#include "entities/obedience.h"
#include "entities/supplication.h"
#include "identity/man.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "values/device_token.h"
#include "values/soul_name.h"


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

private:
	friend class World;

	Testator(id::Man id, id::Soul soul_id, SoulName name, id::Vessel vessel_id, DeviceToken token);
};


} // namespace will::domain
