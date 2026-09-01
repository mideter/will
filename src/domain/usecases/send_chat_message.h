#pragma once

#include "entities/chat_id.h"
#include "entities/user_id.h"
#include "entities/message.h"
#include "entities/participant_id.h"
#include "ports/message_repository.h"
#include "ports/participant_notifier.h"

#include <string_view>


namespace will::domain {


struct SendChatMessageInput {
    UserId user_id;
    ParticipantId sender;
    ChatId chat_id = ChatId::global();
    std::string_view body;
    Timestamp created_at{};
};


class SendChatMessage {
public:
    SendChatMessage(MessageRepository& messages, ParticipantNotifier& notifier);

    Message execute(const SendChatMessageInput& input);

private:
    MessageRepository& messages_;
    ParticipantNotifier& notifier_;
};


} // namespace will::domain
