#pragma once

#include "acts/embodiment.h"
#include "acts/obedience.h"
#include "acts/supplication.h"
#include "beings/deed.h"
#include "beings/witness.h"

#include <memory>
#include <vector>


namespace will::domain {


class Testator;


/// Executor (Послушник) — Исполнитель: исполняет Дело в Послушании.
/// Owns Obedience faces on the heap; Temporality keeps the same places in time.
/// Mode is disclosed in an Obedience; the living heap object is always Testator.
class Executor : public Witness {
public:
	/// Ask testator to become Завещатель; this soul will be the executor.
	/// The pending Supplication is signed by this executor (lodge + Temporality).
	void supplicate(const Testator& testator) const;

	/// Carry out an open deed in an obedience where this soul is executor.
	Deed execute(const Deed& deed) const;

	/// Owned Obedience face by place id. Throws if unknown.
	const Obedience& obedience(id::Obedience id) const;

protected:
	explicit Executor(Embodiment embodiment);

	/// Keep an Obedience face on this executor (signed Supplication / awaken).
	const Obedience& keep(Obedience place) const;

private:
	friend class Supplication;
	friend class World;

	using Witness::say;

	mutable std::vector<std::unique_ptr<Obedience>> obediences_;
};


} // namespace will::domain
