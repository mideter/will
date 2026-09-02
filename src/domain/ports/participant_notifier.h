#pragma once

#include "entities/message.h"


namespace will::domain {


class ParticipantNotifier {
public:
    virtual ~ParticipantNotifier() = default;

    virtual void notify_chat_message(const Message& msg) = 0;
};


} // namespace will::domain
