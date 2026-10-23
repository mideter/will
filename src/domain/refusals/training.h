#pragma once

#include "refusals/refusal.h"


namespace will::domain {


/// What a training refuses.
class TrainOnlyInContemplatedTie final : public Refusal {
public:
	TrainOnlyInContemplatedTie()
		: Refusal("one does a training only in the tie one contemplates")
	{}
};

class OnlyNoviceTrains final : public Refusal {
public:
	OnlyNoviceTrains()
		: Refusal("only the novice of its tie does a training")
	{}
};

class NoSuchExercise final : public Refusal {
public:
	NoSuchExercise()
		: Refusal("no such exercise")
	{}
};

class NoSuchApproach final : public Refusal {
public:
	NoSuchApproach()
		: Refusal("no such approach")
	{}
};

class ApproachAlreadyDone final : public Refusal {
public:
	ApproachAlreadyDone()
		: Refusal("the approach is already done")
	{}
};

class ApproachesInOrder final : public Refusal {
public:
	ApproachesInOrder()
		: Refusal("the approaches of an exercise are done in order")
	{}
};

class AnotherApproachUnderway final : public Refusal {
public:
	AnotherApproachUnderway()
		: Refusal("another approach is underway")
	{}
};

class NoApproachUnderway final : public Refusal {
public:
	NoApproachUnderway()
		: Refusal("no approach of this training is underway")
	{}
};

class TellWhatWasDone final : public Refusal {
public:
	TellWhatWasDone()
		: Refusal("tell first what was done in the approach")
	{}
};

class FinishApproachFirst final : public Refusal {
public:
	FinishApproachFirst()
		: Refusal("finish the approach underway first")
	{}
};


} // namespace will::domain
