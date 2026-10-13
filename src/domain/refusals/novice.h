#pragma once

#include "refusals/refusal.h"


namespace will::domain {


/// What a novice refuses.
class CannotSupplicateOneself final : public Refusal {
public:
	CannotSupplicateOneself()
		: Refusal("cannot supplicate oneself")
	{}
};

class SupplicateOnlyDweller final : public Refusal {
public:
	SupplicateOnlyDweller()
		: Refusal("one supplicates only a dweller of one's abode")
	{}
};

class FulfilOnlyInContemplatedTie final : public Refusal {
public:
	FulfilOnlyInContemplatedTie()
		: Refusal("one fulfils a behest only in the tie one contemplates")
	{}
};

class AlreadyFulfilled final : public Refusal {
public:
	AlreadyFulfilled()
		: Refusal("behest is already executed")
	{}
};

class PlainBehestWithoutExercises final : public Refusal {
public:
	PlainBehestWithoutExercises()
		: Refusal("a plain behest is fulfilled without exercises")
	{}
};


} // namespace will::domain
