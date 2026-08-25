#include "send_chat_message.h"


namespace will::domain {


SendChatMessage::SendChatMessage(MessageRepository& messages, ParticipantNotifier& notifier)
    : messages_(messages)
    , notifier_(notifier)
{}


Message SendChatMessage::execute(const SendChatMessageInput& input)
{
    Message saved = messages_.append(input.chat_id, input.account.user_id, input.body, input.created_at);
    saved.author_name = input.account.name;
    notifier_.notify_chat_message(input.chat_id, saved, input.sender);
    return saved;
}


} // namespace will::domain
