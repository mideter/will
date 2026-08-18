module;

#include <string_view>

export module will.domain.send_chat_message;

export import will.domain.account;
export import will.domain.chat_id;
export import will.domain.message;
export import will.domain.message_repository;
export import will.domain.participant_id;
export import will.domain.participant_notifier;

export namespace will::domain {


struct SendChatMessageInput {
    Account account;
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
