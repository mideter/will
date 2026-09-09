#pragma once

#include "entities/abode.h"
#include "entities/man.h"
#include "ports/temporality.h"
#include "values/word.h"


namespace will::domain {


/// Witness (Свидетель) — living man; source of will and hearing in the waking world.
/// Created in World::accept from an Eternity Man snapshot; stored as Man.
class Witness : public Man {
public:
	Witness(Man&& man, Abode& abode, Temporality& temporality) noexcept;

	Abode& abode() const noexcept { return *abode_; }

	void say(const Word& word) const override;

	Word hear() const override;

private:
	Abode* abode_;
	Temporality* temporality_;
};


} // namespace will::domain
