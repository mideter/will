#pragma once

#include "matter/man.h"
#include "men/soul.h"
#include "men/vessel.h"


namespace will::domain {


class Abode;


class Embodied : public Soul, public Vessel {
public:
	~Embodied() override;

	virtual Abode& abode() const noexcept = 0;

protected:
	explicit Embodied(const matter::Man& kept);
};


} // namespace will::domain
