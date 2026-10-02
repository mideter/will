#pragma once

#include "immanents/obedience.h"
#include "immanents/shepherding.h"
#include "matter/tie.h"


namespace will::domain {


class Deed;
class Novice;
class Testator;


/// Tie (Узы) — one living shared place: both Obedience and Shepherding.
/// Born from matter::Tie; owned on the heap by the Novice as Послушание.
/// Testator shepherds a non-owning Shepherding view. Counterpart sides live in
/// the faces (testator in Obedience, novice in Shepherding); accessors are
/// completed here. Spatiality keeps matter::Tie.
class Tie : public Obedience, public Shepherding {
public:
	explicit Tie(matter::Tie kept);
	~Tie() override;

	Tie(const Tie&) = delete;
	Tie& operator=(const Tie&) = delete;
	Tie(Tie&&) = delete;
	Tie& operator=(Tie&&) = delete;

	const Testator& testator() const override;
	const Novice& novice() const override;

	std::vector<Deed> deeds(const Novice& asker) const override;
};


} // namespace will::domain
