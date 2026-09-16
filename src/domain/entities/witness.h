#pragma once

#include "entities/abode.h"
#include "entities/letter.h"
#include "entities/man.h"
#include "values/word.h"

#include <cstdint>
#include <vector>


namespace will::domain {


/// Witness (Свидетель) — living man; say and retell in the waking world.
/// Focus is Abode. Further modes: Executor (иная воля), Testator (will as own).
/// Heap object is always Testator; Witness is the base living mode.
class Witness : public Man {
public:
	Abode& abode() const noexcept { return *abode_; }

	void say(const Word& word) const override;

	/// Outward witnessing: retell letters fixed in the observed abode (capped).
	std::vector<Letter> retell(std::uint32_t limit) const;

protected:
	Witness(Man&& man, Abode& abode) noexcept;

private:
	Abode* abode_;
};


} // namespace will::domain
