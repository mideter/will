#pragma once

#include "properties/immanent.h"


namespace will::domain {


class Creation;
class Eternity;
class Spatiality;
class Temporality;


/// Life (Жизнь) — proceeds from Eternity, is before the World and brings it
/// forth: Creation is the act of Life, and nothing else creates. Life outlives
/// the World it made. One Life; immanent to Eternity.
class Life : private Immanent<Eternity> {
public:
	/// Throws if no Eternity is realised.
	Life();
	~Life();

	/// Bring forth the World from Eternity and these dimensions, and awaken it.
	Creation create(Spatiality& spatiality, Temporality& temporality);

private:
	static Life* current_;
};


} // namespace will::domain
