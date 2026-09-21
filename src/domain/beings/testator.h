#pragma once

#include "acts/embodiment.h"
#include "acts/shepherding.h"
#include "acts/supplication.h"
#include "beings/novice.h"
#include "beings/deed.h"
#include "values/word.h"

#include <functional>
#include <memory>
#include <vector>


namespace will::domain {


class World;


/// Testator (Завещатель, Тренер) — Novice who may pass received will on as his own.
/// Owns incoming pending Supplications on the heap; living Ties are owned by the
/// Novice as Obedience — this soul reaches them as Shepherding. Temporality keeps
/// the same in time. Only World may birth a Testator onto the heap.
class Testator : public Novice {
public:
	void accept(const Supplication& ask) const;
	void reject(const Supplication& ask) const;
	Deed will(const Shepherding& shepherding, const Word& word) const;

	const Shepherding& shepherding(id::Obedience id) const;
	const Supplication& supplication(const Novice& suppliant) const;
	std::vector<std::reference_wrapper<const Supplication>> supplications() const;

private:
	friend class World;
	friend class Supplication;

	explicit Testator(Embodiment embodiment);

	const Supplication& receive(Supplication supplication) const;
	void drop_supplication(const Novice& suppliant) const;

	mutable std::vector<std::unique_ptr<Supplication>> incoming_;
};


} // namespace will::domain
