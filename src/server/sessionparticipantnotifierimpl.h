#pragma once

#include "ports/participant_notifier.h"


namespace will {


class SessionRegistry;


class SessionParticipantNotifierImpl final : public domain::ParticipantNotifier {
public:
    explicit SessionParticipantNotifierImpl(SessionRegistry& registry);

    void notify_chat_message(domain::ChatId chat, const domain::Message& msg, std::string_view author_name,
                             domain::ParticipantId except_participant) override;

    void send_to_participant(domain::ParticipantId id, const domain::OutboundEvent& ev) override;

private:
    SessionRegistry& registry_;
};


} // namespace will
