#pragma once

#include "beings/heaven.h"
#include "ports/eternity.h"
#include "values/word.h"


namespace will::domain {


class Soul;
class Supplication;


/// Spirit (Дух) — highest foundation of the soul.
/// Will (say) belongs to spirit; living modes reveal it (speech, later bequest/deed).
/// Hearing is not a separate act — the Witness himself is the focus.
/// Knows Heaven via heaven(). Living Man presents the soul to Heaven at birth.
/// Neither copy nor transfer; only Soul may bring forth Spirit.
class Spirit {
public:
	virtual ~Spirit() = default;

	Spirit(const Spirit&) = delete;
	Spirit& operator=(const Spirit&) = delete;
	Spirit(Spirit&&) = delete;
	Spirit& operator=(Spirit&&) = delete;

	/// Act of will: speak a word. Outside the living world this fails; Witness reveals it.
	virtual void say(const Word& word) const;

protected:
	friend class Supplication;

	Spirit() = default;

	/// The one living Heaven (static so Soul::of can look up without an instance).
	static Heaven& heaven() { return Heaven::the(); }

	/// Eternity reached from Heaven (Spirit is friend of Heaven).
	static Eternity& eternity() { return heaven().eternity(); }

	/// Present this living soul to Heaven (heap-stable address). Used by Man at birth.
	void present() const;
};


} // namespace will::domain
