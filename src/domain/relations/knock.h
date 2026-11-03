#pragma once

#include "relations/contemplation.h"


namespace will::domain {


class Gates;


class Knock final : public Contemplation {
public:
	Knock(Birth<Heaven> birth, const Man& who, const Gates& gates);
};


} // namespace will::domain
