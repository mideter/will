#pragma once

#include "entities/earth.h"


namespace will::domain {


/// Dust (Прах) — earthly foundation of the vessel; Dust is Earth (protected).
class Dust : protected Earth {
public:
	bool operator==(const Dust&) const = default;
};


} // namespace will::domain
