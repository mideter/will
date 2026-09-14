#pragma once

#include "entities/abode.h"
#include "entities/man.h"
#include "values/word.h"


namespace will::domain {


/// Witness (Свидетель) — living man; source of will in the waking world.
/// Created in World::accept from an Eternity Man snapshot; stored as Man.
/// Focus is Abode; hearing is the Witness himself. Saying goes through Dust (Earth).
class Witness : public Man {
public:
	Witness(Man&& man, Abode& abode) noexcept;

	Abode& abode() const noexcept { return *abode_; }

	void say(const Word& word) const override;

private:
	Abode* abode_;
};


} // namespace will::domain
