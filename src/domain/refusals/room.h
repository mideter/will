#pragma once

#include "refusals/refusal.h"


namespace will::domain {


/// What the gates, the upper room and a room refuses.
class KeepGatesByKind final : public Refusal {
public:
	KeepGatesByKind()
		: Refusal("one keeps the gates only by one's kind")
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

class BearOnlyAtKeptGates final : public Refusal {
public:
	BearOnlyAtKeptGates()
		: Refusal("one bears only at gates one keeps")
	{}
};

class RegardNotOneself final : public Refusal {
public:
	RegardNotOneself()
		: Refusal("one does not regard oneself")
	{}
};

class DoesNotEnterRoom final : public Refusal {
public:
	DoesNotEnterRoom()
		: Refusal("Witness does not enter this room")
	{}
};


} // namespace will::domain
