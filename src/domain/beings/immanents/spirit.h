#pragma once

#include "acts/contemplation.h"
#include "beings/heaven.h"
#include "ports/eternity.h"
#include "properties/immanent.h"
#include "values/saying.h"


namespace will::domain {


class Abode;
class Soul;


/// Spirit (Дух) — highest foundation of the soul; immanent to Heaven.
/// Will (say) belongs to spirit; living modes reveal it (speech, later bequest/deed).
/// Knows Heaven via heaven(). Living Man presents the soul to Heaven at birth.
/// Neither copy nor transfer; only Soul may bring forth Spirit.
class Spirit : public Immanent<Heaven> {
public:
	virtual ~Spirit() = default;

	/// Outside the living world this fails; Witness reveals it.
	virtual void say(const Saying& saying) const;

protected:
	Spirit() = default;

	/// Static so Soul::of can look up without an instance.
	static Heaven& heaven() { return Heaven::the(); }

	static Eternity& eternity() { return heaven().eternity(); }

	void contemplate(const Abode& abode) const;

	const Contemplation& contemplation() const;
};


} // namespace will::domain
