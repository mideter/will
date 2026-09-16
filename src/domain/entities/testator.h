#pragma once

#include "entities/executor.h"
#include "entities/man.h"


namespace will::domain {


/// Testator (Завещатель) — Executor who may pass received will on as his own.
/// All living modes are present from the start; situations disclose them.
/// World::accept always places a Testator on the heap.
class Testator : public Executor {
public:
	explicit Testator(Man&& man);
};


} // namespace will::domain
