#pragma once

#include "ports/participant_notifier.h"
#include "ports/user_repository.h"


namespace will {


class SessionRegistry;


class SessionParticipantNotifierImpl final : public domain::ParticipantNotifier {
public:
    SessionParticipantNotifierImpl(SessionRegistry& registry, domain::UserRepository& users);

    void notify_letter(const domain::Letter& letter) override;

private:
    SessionRegistry& registry_;
    domain::UserRepository& users_;
};


} // namespace will
