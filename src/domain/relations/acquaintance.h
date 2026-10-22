#pragma once

#include "properties/birth.h"
#include "properties/immanent.h"


namespace will::domain {


class Abode;
class Inner;
class Man;
class Outer;
class Space;


/// Acquaintance (Знакомый) — every dweller is at least one: he enters neither part
/// and only knows that he dwells there. Regarded anew, he is born anew of the new kind.
class Acquaintance : public Immanent<Space> {
public:
	Acquaintance(Birth<Abode>, const Man& man);
	virtual ~Acquaintance() = default;

	const Man& man() const noexcept { return man_; }

	virtual bool enters(const Outer& part) const;
	virtual bool enters(const Inner& part) const;

private:
	const Man& man_;
};


} // namespace will::domain
