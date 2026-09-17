#pragma once

#include "acts/embodiment.h"
#include "beings/abode.h"
#include "beings/letter.h"
#include "beings/man.h"
#include "values/word.h"

#include <cstdint>
#include <memory>
#include <vector>


namespace will::domain {


/// Witness (Свидетель) — living man; say and retell in the waking world.
/// Owns his personal Abode (id = man id, name from soul). Further modes: Executor, Testator.
/// Heap object is always Testator; Witness is the base living mode.
class Witness : public Man {
public:
	Abode& abode() const noexcept { return *abode_; }

	void say(const Word& word) const override;

	/// Outward witnessing: retell letters of authors known to Heaven (capped).
	std::vector<Letter> retell(std::uint32_t limit) const;

protected:
	explicit Witness(Embodiment embodiment);

private:
	std::unique_ptr<Abode> abode_;
};


} // namespace will::domain
