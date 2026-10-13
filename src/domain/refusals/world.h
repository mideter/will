#pragma once

#include "refusals/refusal.h"


namespace will::domain {


/// What the world refuses.
class NoSuchUnborn final : public Refusal {
public:
	NoSuchUnborn()
		: Refusal("no such unborn")
	{}
};

class NotOwnFather final : public Refusal {
public:
	NotOwnFather()
		: Refusal("one is not one's own father")
	{}
};

class NotDescendantAsFather final : public Refusal {
public:
	NotDescendantAsFather()
		: Refusal("one does not choose a descendant by spirit as one's father")
	{}
};


} // namespace will::domain
