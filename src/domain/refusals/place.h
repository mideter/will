#pragma once

#include "refusals/refusal.h"


namespace will::domain {


/// Where one stands: what is done in a place of a kind, is done only standing in it.
class StandInRoom final : public Refusal {
public:
	StandInRoom()
		: Refusal("one does this only standing in a room")
	{}
};

class StandAtHelm final : public Refusal {
public:
	StandAtHelm()
		: Refusal("one does this only standing at a helm")
	{}
};

class StandAtGates final : public Refusal {
public:
	StandAtGates()
		: Refusal("one does this only standing at gates")
	{}
};

/// Where one goes: out of an abode and into another only through gates.
class PassThroughGates final : public Refusal {
public:
	PassThroughGates()
		: Refusal("one goes there only through gates")
	{}
};


} // namespace will::domain
