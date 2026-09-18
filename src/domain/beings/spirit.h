#pragma once

#include "beings/heaven.h"
#include "values/word.h"


namespace will::domain {


class Soul;


/// Spirit (Дух) — highest foundation of the soul.
/// Will (say) belongs to spirit; living modes reveal it (speech, later bequest/deed).
/// Hearing is not a separate act — the Witness himself is the focus.
/// Knows Heaven via the(). Living Man presents the soul to Heaven at birth.
/// Neither copy nor transfer.
class Spirit {
public:
	Spirit() = default;
	virtual ~Spirit() = default;

	Spirit(const Spirit&) = delete;
	Spirit& operator=(const Spirit&) = delete;
	Spirit(Spirit&&) = delete;
	Spirit& operator=(Spirit&&) = delete;

	Heaven& heaven() const { return Heaven::the(); }

	/// Act of will: speak a word. Outside the living world this fails; Witness reveals it.
	virtual void say(const Word& word) const;

protected:
	/// Present this living soul to Heaven (heap-stable address). Used by Man at birth.
	void present() const;

	/// The one living Heaven (for static lookup from Soul).
	static Heaven& the_heaven() { return Heaven::the(); }
};


} // namespace will::domain
