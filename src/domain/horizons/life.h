#pragma once

#include "properties/immanent.h"


namespace will::domain {


class Creation;
class Eternity;
class Spatiality;
class Temporality;


/// Life (Жизнь) — that which is before the World and brings it forth: Creation
/// is the act of Life, and nothing else creates. Life outlives the World it made.
/// One Life; immanent to itself.
class Life : private Immanent<Life> {
public:
	Life();
	~Life();

	/// Bring forth the World and awaken it.
	Creation create(Eternity& eternity, Spatiality& spatiality, Temporality& temporality);

private:
	static Life* current_;
};


} // namespace will::domain
