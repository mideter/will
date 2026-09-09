#pragma once

#include "values/word.h"


namespace will::domain {


/// Spirit (Дух) — highest, inseparable foundation of the soul.
/// Will (say) and hearing (hear) belong to spirit; only a living Witness enacts them.
class Spirit {
public:
	virtual ~Spirit() = default;

	/// Act of will: speak a word. Outside the living world this fails; Witness overrides.
	virtual void say(const Word& word) const;

	/// Act of hearing: one word. Outside the living world this fails; Witness overrides.
	virtual Word hear() const;

	bool operator==(const Spirit&) const = default;
};


} // namespace will::domain
