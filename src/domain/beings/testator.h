#pragma once

#include "acts/embodiment.h"
#include "acts/obedience.h"
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
/// Owns Shepherding (Ведение) faces and incoming pending Supplications on the heap;
/// Temporality keeps the same in time. Only World may birth a Testator onto the heap.
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

	const Shepherding& keep(Shepherding place) const;
	const Supplication& receive(Supplication supplication) const;
	void drop_supplication(const Novice& suppliant) const;

	mutable std::vector<std::unique_ptr<Shepherding>> shepherdings_;
	mutable std::vector<std::unique_ptr<Supplication>> incoming_;
};


} // namespace will::domain
