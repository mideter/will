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

class StandInUpperRoom final : public Refusal {
public:
	StandInUpperRoom()
		: Refusal("one does this only standing in an upper room")
	{}
};

class StandAtGates final : public Refusal {
public:
	StandAtGates()
		: Refusal("one does this only standing at gates")
	{}
};


} // namespace will::domain
