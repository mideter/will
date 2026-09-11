#pragma once

#include "entities/heaven.h"
#include "values/word.h"


namespace will::domain {


/// Spirit (Дух) — highest foundation of the soul; Spirit is Heaven.
/// Will (say) and hearing (hear) belong to spirit; only a living Witness enacts them.
class Spirit : public Heaven {
public:
	virtual ~Spirit() = default;

	/// Act of will: speak a word. Outside the living world this fails; Witness overrides.
	virtual void say(const Word& word) const;

	/// Act of hearing: one word. Outside the living world this fails; Witness overrides.
	virtual Word hear() const;

	bool operator==(const Spirit&) const = default;
};


} // namespace will::domain
