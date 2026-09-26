#pragma once

#include "beings/immanents/obedience.h"
#include "beings/immanents/shepherding.h"
#include "acts/tying.h"


namespace will::domain {


class Novice;
class Testator;


/// Tie (Узы) — one living shared place: both Obedience and Shepherding.
/// Born from Tying (Связывание); owned on the heap by the Novice as Послушание.
/// Testator keeps a non-owning Shepherding view. Counterpart sides live in
/// the faces (testator in Obedience, novice in Shepherding); accessors are
/// completed here. Temporality keeps Tying in time.
class Tie : public Obedience, public Shepherding {
public:
	explicit Tie(Tying tying);
	~Tie() override;

	Tie(const Tie&) = delete;
	Tie& operator=(const Tie&) = delete;
	Tie(Tie&&) = delete;
	Tie& operator=(Tie&&) = delete;

	const Testator& testator() const override;
	const Novice& novice() const override;
};


} // namespace will::domain
