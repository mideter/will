#pragma once

#include "refusals/refusal.h"


namespace will::domain {


/// What an abode refuses.
class AlreadyDwells final : public Refusal {
public:
	AlreadyDwells()
		: Refusal("he already dwells in this abode")
	{}
};

class DoesNotDwell final : public Refusal {
public:
	DoesNotDwell()
		: Refusal("one does not dwell in this abode")
	{}
};

class OnlyHostWrites final : public Refusal {
public:
	OnlyHostWrites()
		: Refusal("only the host writes in his abode")
	{}
};

class FatherByFleshNeighbour final : public Refusal {
public:
	FatherByFleshNeighbour()
		: Refusal("the father by flesh is ever at least a neighbour")
	{}
};

class FatherBySpiritFriend final : public Refusal {
public:
	FatherBySpiritFriend()
		: Refusal("the father by spirit is ever a friend")
	{}
};


} // namespace will::domain
