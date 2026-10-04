#pragma once

#include "properties/immanent.h"


namespace will::domain {


class Creation;
class Eternity;
class Spatiality;
class Temporality;


/// Life (Жизнь) — proceeds from Eternity, is before the World and brings it
/// forth: Creation is the act of Life, and nothing else creates. Life outlives
/// the World it made. It holds Spatiality and Temporality and gives them to the
/// World at Creation. One Life; immanent to Eternity.
class Life : private Immanent<Eternity> {
public:
	/// Throws if no Eternity is realised.
	Life(Spatiality& spatiality, Temporality& temporality);
	~Life();

	/// Bring forth the World from Eternity and the dimensions Life holds, and awaken it.
	Creation create();

private:
	static Life* current_;

	Spatiality& spatiality_;
	Temporality& temporality_;
};


} // namespace will::domain
