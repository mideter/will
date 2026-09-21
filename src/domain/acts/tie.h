#pragma once

#include "acts/obedience.h"
#include "acts/shepherding.h"
#include "acts/tying.h"
#include "identity/obedience.h"


namespace will::domain {


class Novice;
class Testator;


/// Tie (Узы) — one living shared place: both Obedience and Shepherding.
/// Born from Tying (Связывание); owned on the heap by the Novice as Послушание.
/// Testator keeps a non-owning Shepherding view. Sides live only here;
/// Temporality keeps Tying in time.
class Tie : public Obedience, public Shepherding {
public:
	explicit Tie(Tying tying);
	~Tie() override;

	Tie(const Tie&) = delete;
	Tie& operator=(const Tie&) = delete;
	Tie(Tie&&) = delete;
	Tie& operator=(Tie&&) = delete;

	using Obedience::obedience_id;

	const Testator& testator() const override;
	const Novice& novice() const override;

private:
	const Testator& testator_;
	const Novice& novice_;
};


} // namespace will::domain
