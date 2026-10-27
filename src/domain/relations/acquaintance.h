#pragma once

#include "properties/birth.h"
#include "properties/immanent.h"


namespace will::domain {


class Abode;
class Man;
class OuterAbode;
class Space;


/// Every dweller is at least one: he sees no room of the abode, neither outward nor
/// inward, and only knows that he dwells there. Regarded anew, he is born anew of the new
/// kind.
class Acquaintance : public Immanent<Space> {
public:
	Acquaintance(Birth<OuterAbode>, const Man& man);
	virtual ~Acquaintance() = default;

	const Man& man() const noexcept { return man_; }

	virtual bool sees(const OuterAbode& outer) const;
	virtual bool sees(const Abode& abode) const;

private:
	const Man& man_;
};


} // namespace will::domain
