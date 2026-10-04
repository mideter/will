#pragma once

#include "properties/immanent.h"


namespace will::domain {


class Eternity;


/// Word (Слово) — that which the spirit utters; ontological role, immanent to
/// Eternity: there it is uttered and given its identity, and that part of it
/// never changes. Neither copied nor moved.
/// No identity and no Saying here. Living Letter and Deed are Word in the world.
class Word : public Immanent<Eternity> {
public:
	virtual ~Word() = default;

protected:
	Word() = default;
};


} // namespace will::domain
