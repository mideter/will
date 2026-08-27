#pragma once

#include "entities/message.h"
#include "entities/participant_id.h"


namespace will::domain {


class ParticipantNotifier {
public:
    virtual ~ParticipantNotifier() = default;

    virtual void notify_chat_message(const Message& msg, ParticipantId except_participant) = 0;
};


} // namespace will::domain
