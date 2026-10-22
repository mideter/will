#pragma once

#include "matter/man.h"
#include "men/soul.h"
#include "men/vessel.h"


namespace will::domain {


class Abode;


/// A soul in a body, with its abode: what the roles of a man
/// stand on. The man builds it; a role is never born alone.
class Embodied : public Soul, public Vessel {
public:
	~Embodied() override;

	virtual Abode& abode() const noexcept = 0;

protected:
	explicit Embodied(const matter::Man& kept);
};


} // namespace will::domain
