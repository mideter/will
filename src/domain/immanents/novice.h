#pragma once

#include "matter/man.h"
#include "immanents/obedience.h"
#include "acts/supplication.h"
#include "matter/tie.h"
#include "words/behest.h"
#include "words/deed.h"
#include "values/saying.h"
#include "immanents/witness.h"
#include "identity/word.h"

#include <functional>
#include <memory>
#include <optional>
#include <vector>


namespace will::domain {


class Testator;


/// Novice (Послушник) — Исполнитель: исполняет Веление в Послушании.
/// Owns living Ties as Obedience (Послушание) on the heap; Spatiality keeps
/// their matter::Tie. Mode is disclosed in an Obedience; the living heap object
/// is always Testator.
class Novice : public Witness {
public:
	/// Ask addressee to become Завещатель; this soul will be the novice.
	/// The pending Supplication is signed by this novice (lodge + Temporality).
	void supplicate(const Testator& addressee) const;

	/// Fulfil a behest of a tie where this soul is novice, with a report or,
	/// without one, «совершено», while contemplating that tie. Throws if the
	/// behest is already fulfilled or one looks elsewhere.
	std::shared_ptr<const Deed> execute(const Behest& behest,
										std::optional<Saying> report = std::nullopt) const;

	bool follows(const Testator& testator) const noexcept;

	std::vector<std::reference_wrapper<const Obedience>> obediences() const;

	/// Owned Obedience face under this testator. Throws if unknown.
	const Obedience& obedience(const Testator& testator) const;

protected:
	explicit Novice(matter::Man kept);

private:
	friend class Supplication;
	friend class World;

	using Witness::say;

	const Obedience& follow(matter::Tie kept) const;
	const Obedience& follow(std::unique_ptr<Obedience> place) const;

	mutable std::vector<std::unique_ptr<Obedience>> obediences_;
};


} // namespace will::domain
