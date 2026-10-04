#pragma once

#include "immanents/obedience.h"
#include "immanents/shepherding.h"
#include "matter/tie.h"

#include <mutex>
#include "properties/birth.h"


namespace will::domain {


class Behest;
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
	Tie(Birth<Novice>, matter::Tie kept);
	~Tie() override;

	Tie(const Tie&) = delete;
	Tie& operator=(const Tie&) = delete;
	Tie(Tie&&) = delete;
	Tie& operator=(Tie&&) = delete;

	const Testator& testator() const override;
	const Novice& novice() const override;

	std::vector<std::shared_ptr<const Behest>> behests(const Novice& asker) const override;
	std::vector<std::shared_ptr<const Deed>> deeds(const Novice& asker) const override;

	std::shared_ptr<const Deed> inscribe(Birth<Novice>, matter::Deed kept) const override;
	std::shared_ptr<const Behest> inscribe(Birth<Testator>, matter::Behest kept) const override;

	/// Its two sides dwell in a tie.
	bool dwells(const Man& man) const override;

	/// The behests and deeds placed here, oldest first, for a gaze upon this tie.
	/// The ones still living are shared; the rest are born from the matter Life gives.
	std::vector<std::shared_ptr<const Word>> words(const Contemplation& gaze) const override;

private:
	std::vector<std::shared_ptr<const Word>> living() const;

	mutable std::mutex mutex_;
};


} // namespace will::domain
