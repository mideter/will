#pragma once

#include "matter/man.h"
#include "matter/supplication.h"
#include "properties/birth.h"
#include "places/shepherding.h"
#include "relations/supplication.h"
#include "men/novice.h"
#include "words/behest.h"
#include "values/saying.h"

#include <functional>
#include <memory>
#include <vector>


namespace will::domain {


class World;


/// Testator (Завещатель, Тренер) — Novice who may pass received will on as his own.
/// Owns incoming pending Supplications on the heap; living Ties are owned by the
/// Novice as Obedience — this soul shepherds those Ties through non-owning
/// Shepherding views. Temporality keeps matter::Supplication, Spatiality matter::Tie. Pending Supplications
/// to this soul are kept by Heaven. Only World may birth a Testator onto the heap.
class Testator : public Novice {
public:
	Testator(Birth<World>, matter::Man kept);

	/// The Shepherding of the Tie born from the accepted supplication.
	const Shepherding& accept(const Supplication& ask) const;
	void reject(const Supplication& ask) const;

	/// Will in a tie one contemplates. Throws if one looks elsewhere.
	std::shared_ptr<const Behest> will(const Shepherding& shepherding, const Saying& saying) const;

	/// Shepherding of the Tie with this novice. Throws if unknown.
	const Shepherding& shepherding(const Novice& novice) const;
	/// The supplication of this suppliant awaiting this soul's answer. Throws if none.
	std::shared_ptr<const Supplication> supplication(const Novice& suppliant) const;
	std::vector<std::shared_ptr<const Supplication>> supplications() const;

	/// Hear a supplication a novice submits, or one the World recalls on
	/// awakening; Heaven keeps it until it is answered. One already heard from
	/// the same suppliant, or from one already shepherded, is not kept twice.
	void hear(Birth<Novice> birth, matter::Supplication kept) const;
	void hear(Birth<World> birth, matter::Supplication kept) const;

private:
	friend class World;
	friend class Supplication;
	friend class Tie;

	const Shepherding& shepherd(const Shepherding& place) const;
	void release(const Shepherding& place) const;

	void hold(std::shared_ptr<const Supplication> supplication) const;
	void drop(const Supplication& answered) const;

	mutable std::vector<const Shepherding*> shepherdings_;
};


} // namespace will::domain
