#pragma once

#include "entities/heaven.h"
#include "values/word.h"


namespace will::domain {


/// Spirit (Дух) — highest foundation of the soul; Spirit is Heaven.
/// Will (say) and outward retelling belong to spirit; only a living Witness enacts them.
/// Hearing is not a separate act — the Witness himself is the focus.
class Spirit : public Heaven {
public:
	virtual ~Spirit() = default;

	/// Act of will: speak a word. Outside the living world this fails; Witness overrides.
	virtual void say(const Word& word) const;

	bool operator==(const Spirit&) const = default;
};


} // namespace will::domain
