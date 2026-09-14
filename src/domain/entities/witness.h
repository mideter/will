#pragma once

#include "entities/abode.h"
#include "entities/letter.h"
#include "entities/man.h"
#include "values/word.h"

#include <cstdint>
#include <vector>


namespace will::domain {


/// Witness (Свидетель) — living man; source of will in the waking world.
/// Created in World::accept from an Eternity Man snapshot; stored as Man.
/// Focus is Abode. Witnesses both ways: say inward, retell outward.
class Witness : public Man {
public:
	Witness(Man&& man, Abode& abode) noexcept;

	Abode& abode() const noexcept { return *abode_; }

	void say(const Word& word) const override;

	/// Outward witnessing: retell letters fixed in the observed abode (capped).
	std::vector<Letter> retell(std::uint32_t limit) const;

private:
	Abode* abode_;
};


} // namespace will::domain
