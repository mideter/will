#pragma once

#include "matter/man.h"
#include "properties/birth.h"
#include "immanents/shepherding.h"
#include "acts/supplication.h"
#include "immanents/novice.h"
#include "words/deed.h"
#include "values/saying.h"

#include <functional>
#include <memory>
#include <vector>


namespace will::domain {


class World;


/// Testator (Завещатель, Тренер) — Novice who may pass received will on as his own.
/// Owns incoming pending Supplications on the heap; living Ties are owned by the
/// Novice as Obedience — this soul shepherds those Ties through non-owning
/// Shepherding views. Temporality keeps matter::Supplication, Spatiality matter::Tie. Only World may birth
/// a Testator onto the heap.
class Testator : public Novice {
public:
	Testator(Birth<World>, matter::Man kept);

	/// The Shepherding of the Tie born from the accepted supplication.
	const Shepherding& accept(const Supplication& ask) const;
	void reject(const Supplication& ask) const;

	Deed will(const Shepherding& shepherding, const Saying& saying) const;

	/// Shepherding of the Tie with this novice. Throws if unknown.
	const Shepherding& shepherding(const Novice& novice) const;
	const Supplication& supplication(const Novice& suppliant) const;
	std::vector<std::reference_wrapper<const Supplication>> supplications() const;

private:
	friend class World;
	friend class Supplication;
	friend class Tie;

	const Shepherding& shepherd(const Shepherding& place) const;
	void release(const Shepherding& place) const;

	const Supplication& receive(Supplication supplication) const;
	void drop(const Supplication& ask) const;

	mutable std::vector<const Shepherding*> shepherdings_;
	mutable std::vector<std::unique_ptr<Supplication>> incoming_;
};


} // namespace will::domain
