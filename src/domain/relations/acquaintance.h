#pragma once

#include "properties/birth.h"
#include "properties/immanent.h"


namespace will::domain {


class Abode;
class Man;
class OuterAbode;
class Space;


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
