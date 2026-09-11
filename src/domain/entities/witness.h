#pragma once

#include "entities/abode.h"
#include "entities/man.h"
#include "values/word.h"


namespace will::domain {


/// Witness (Свидетель) — living man; source of will and hearing in the waking world.
/// Created in World::accept from an Eternity Man snapshot; stored as Man.
/// Focus is Abode; fixing/hearing go through Dust (Earth) inherited by Man.
class Witness : public Man {
public:
	Witness(Man&& man, Abode& abode) noexcept;

	Abode& abode() const noexcept { return *abode_; }

	void say(const Word& word) const override;

	Word hear() const override;

private:
	Abode* abode_;
};


} // namespace will::domain
