#pragma once

#include "ports/participant_notifier.h"


namespace will {


class SessionRegistry;


class SessionParticipantNotifierImpl final : public domain::ParticipantNotifier {
public:
    explicit SessionParticipantNotifierImpl(SessionRegistry& registry);

    void notify_chat_message(const domain::Message& msg, std::string_view author_name,
                             domain::ParticipantId except_participant) override;

private:
    SessionRegistry& registry_;
};


} // namespace will
