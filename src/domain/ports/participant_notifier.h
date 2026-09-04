#pragma once

#include "entities/letter.h"


namespace will::domain {


class ParticipantNotifier {
public:
	virtual ~ParticipantNotifier() = default;

	virtual void notify_letter(const Letter& letter) = 0;
};


} // namespace will::domain
