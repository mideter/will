#pragma once

#include "ports/participant_notifier.h"


namespace will {


class TcpConnectionRegistry;


class TcpConnectionParticipantNotifierImpl final : public domain::ParticipantNotifier {
public:
    explicit TcpConnectionParticipantNotifierImpl(TcpConnectionRegistry& registry);

    void notify_chat_message(domain::ChatId chat, const domain::Message& msg,
                             domain::ParticipantId except_participant) override;

    void send_to_participant(domain::ParticipantId id, const domain::OutboundEvent& ev) override;

private:
    TcpConnectionRegistry& registry_;
};


} // namespace will
