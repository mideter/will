#pragma once

#include "refusals/refusal.h"


namespace will::domain {


/// Where one stands: what is done in a room of a kind, is done only standing in it.
class StandInCell final : public Refusal {
public:
	StandInCell()
		: Refusal("one does this only standing in a cell")
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
