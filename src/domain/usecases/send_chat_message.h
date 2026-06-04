#pragma once

#include "entities/account.h"
#include "entities/chat_id.h"
#include "entities/message.h"
#include "entities/participant_id.h"
#include "ports/message_repository.h"
#include "ports/participant_notifier.h"

#include <string_view>


namespace will::domain {


struct SendChatMessageInput {
    Account account;
    ParticipantId sender;
    ChatId chat_id = ChatId::global();
    std::string_view body;
    TimestampMs created_at = 0;
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
