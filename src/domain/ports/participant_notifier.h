#pragma once

#include "entities/message.h"
#include "entities/participant_id.h"

#include <string_view>


namespace will::domain {


class ParticipantNotifier {
public:
    virtual ~ParticipantNotifier() = default;

    virtual void notify_chat_message(const Message& msg, std::string_view author_name,
                                     ParticipantId except_participant) = 0;
};


} // namespace will::domain
