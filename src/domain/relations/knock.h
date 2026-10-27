#pragma once

#include "relations/contemplation.h"


namespace will::domain {


class Gates;


/// A gaze upon gates one does not dwell behind that calls their host: it lasts while one
/// stands there, and fades as one turns away.
class Knock final : public Contemplation {
public:
	Knock(Birth<Heaven> birth, const Man& who, const Gates& gates);
};


} // namespace will::domain
