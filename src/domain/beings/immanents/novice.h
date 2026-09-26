#pragma once

#include "acts/embodiment.h"
#include "beings/immanents/obedience.h"
#include "acts/supplication.h"
#include "acts/tying.h"
#include "beings/deed.h"
#include "beings/immanents/witness.h"

#include <memory>
#include <vector>


namespace will::domain {


class Testator;


/// Novice (Послушник) — Исполнитель: исполняет Дело в Послушании.
/// Owns living Ties as Obedience (Послушание) on the heap; Temporality keeps
/// Tyings in time. Mode is disclosed in an Obedience; the living heap object
/// is always Testator.
class Novice : public Witness {
public:
	/// Ask addressee to become Завещатель; this soul will be the novice.
	/// The pending Supplication is signed by this novice (lodge + Temporality).
	void supplicate(const Testator& addressee) const;

	/// Carry out an open deed in an obedience where this soul is novice.
	Deed execute(const Deed& deed) const;

	/// Owned Obedience face by tie id. Throws if unknown.
	const Obedience& obedience(id::Tie id) const;

protected:
	explicit Novice(Embodiment embodiment);

private:
	friend class Supplication;
	friend class World;

	using Witness::say;

	const Obedience& follow(Tying tying) const;
	const Obedience& follow(std::unique_ptr<Obedience> place) const;

	mutable std::vector<std::unique_ptr<Obedience>> obediences_;
};


} // namespace will::domain
