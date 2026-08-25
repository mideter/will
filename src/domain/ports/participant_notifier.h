#pragma once

#include "entities/chat_id.h"
#include "entities/message.h"
#include "entities/participant_id.h"
#include "events/outbound_event.h"


namespace will::domain {


class ParticipantNotifier {
public:
    virtual ~ParticipantNotifier() = default;

    virtual void notify_chat_message(ChatId chat, const Message& msg, ParticipantId except_participant) = 0;
    virtual void send_to_participant(ParticipantId id, const OutboundEvent& ev) = 0;
};


} // namespace will::domain
