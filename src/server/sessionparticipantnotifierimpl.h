#pragma once

#include "ports/participant_notifier.h"
#include "entities/heaven.h"


namespace will {


class SessionRegistry;


class SessionParticipantNotifierImpl final : public domain::ParticipantNotifier {
public:
	SessionParticipantNotifierImpl(SessionRegistry& registry, domain::Heaven& heaven);

	void notify_letter(const domain::Letter& letter) override;

private:
	SessionRegistry& registry_;
	domain::Heaven& heaven_;
};


} // namespace will
