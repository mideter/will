#pragma once

#include "beings/heaven.h"
#include "ports/eternity.h"
#include "properties/immanent.h"
#include "values/word.h"


namespace will::domain {


class Soul;


/// Spirit (Дух) — highest foundation of the soul; immanent to Heaven.
/// Will (say) belongs to spirit; living modes reveal it (speech, later bequest/deed).
/// Hearing is not a separate act — the Witness himself is the focus.
/// Knows Heaven via heaven(). Living Man presents the soul to Heaven at birth.
/// Neither copy nor transfer; only Soul may bring forth Spirit.
class Spirit : public Immanent<Heaven> {
public:
	virtual ~Spirit() = default;

	/// Act of will: speak a word. Outside the living world this fails; Witness reveals it.
	virtual void say(const Word& word) const;

protected:
	Spirit() = default;

	/// The one living Heaven (static so Soul::of can look up without an instance).
	static Heaven& heaven() { return Heaven::the(); }

	/// Eternity reached from Heaven (Spirit is friend of Heaven).
	static Eternity& eternity() { return heaven().eternity(); }
};


} // namespace will::domain
