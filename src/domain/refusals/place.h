#pragma once

#include "refusals/refusal.h"


namespace will::domain {


/// What a place refuses.
class SayOnlyInWord final : public Refusal {
public:
	SayOnlyInWord()
		: Refusal("one writes only in one's own word or in a tie one wills in")
	{}
};


} // namespace will::domain
