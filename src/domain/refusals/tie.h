#pragma once

#include "refusals/refusal.h"


namespace will::domain {


/// What a tie refuses.
class OnlyTestatorWills final : public Refusal {
public:
	OnlyTestatorWills()
		: Refusal("only the testator of a tie wills in it")
	{}
};

class OnlyNoviceFulfils final : public Refusal {
public:
	OnlyNoviceFulfils()
		: Refusal("only the novice of a tie does a deed in it")
	{}
};

class TieOnlyMutualDwellers final : public Refusal {
public:
	TieOnlyMutualDwellers()
		: Refusal("a tie binds only those who dwell in each other's abodes")
	{}
};

class AlreadyTied final : public Refusal {
public:
	AlreadyTied()
		: Refusal("obedience already exists for this pair")
	{}
};


} // namespace will::domain
