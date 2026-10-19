#pragma once

#include "refusals/refusal.h"


namespace will::domain {


/// What the gates, the upper room and a room refuses.
class OnlyHostActs final : public Refusal {
public:
	OnlyHostActs()
		: Refusal("only the host acts here; others but look")
	{}
};

class NotAtGates final : public Refusal {
public:
	NotAtGates()
		: Refusal("he does not stand at the gates")
	{}
};

class BearOnlyAtGates final : public Refusal {
public:
	BearOnlyAtGates()
		: Refusal("one bears only standing in gates")
	{}
};

class DoesNotEnterRoom final : public Refusal {
public:
	DoesNotEnterRoom()
		: Refusal("Witness does not enter this room")
	{}
};


} // namespace will::domain
