#pragma once

#include "ports/participant_notifier.h"
#include "ports/god_repository.h"


namespace will {


class SessionRegistry;


class SessionParticipantNotifierImpl final : public domain::ParticipantNotifier {
public:
    SessionParticipantNotifierImpl(SessionRegistry& registry, domain::GodRepository& gods);

    void notify_letter(const domain::Letter& letter) override;

private:
    SessionRegistry& registry_;
    domain::GodRepository& gods_;
};


} // namespace will
