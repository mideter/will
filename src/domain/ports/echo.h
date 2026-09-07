#pragma once

#include "entities/letter.h"


namespace will::domain {


/// Echo — live presence of communion; carries inscribed letters to participants.
class Echo {
public:
	virtual ~Echo() = default;

	virtual void notify_letter(const Letter& letter) = 0;
};


} // namespace will::domain
