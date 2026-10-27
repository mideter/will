#pragma once

#include "refusals/refusal.h"


namespace will::domain {


/// What the gates, the helm and a room refuses.
class OnlyHostActs final : public Refusal {
public:
	OnlyHostActs()
		: Refusal("only the host acts here; others but look")
	{}
};

class DoesNotKnock final : public Refusal {
public:
	DoesNotKnock()
		: Refusal("he does not knock at the gates")
	{}
};

class AlreadyKnocks final : public Refusal {
public:
	AlreadyKnocks()
		: Refusal("one already knocks at these gates")
	{}
};

class DoesNotEnterRoom final : public Refusal {
public:
	DoesNotEnterRoom()
		: Refusal("Witness does not enter this room")
	{}
};


} // namespace will::domain
