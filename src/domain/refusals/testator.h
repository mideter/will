#pragma once

#include "refusals/refusal.h"


namespace will::domain {


/// What a testator refuses.
class HearOnlyDweller final : public Refusal {
public:
	HearOnlyDweller()
		: Refusal("one hears only a dweller of one's abode")
	{}
};

class UnknownSupplication final : public Refusal {
public:
	UnknownSupplication()
		: Refusal("unknown supplication")
	{}
};

class WillOnlyInContemplatedTie final : public Refusal {
public:
	WillOnlyInContemplatedTie()
		: Refusal("one wills only in the tie one contemplates")
	{}
};


} // namespace will::domain
