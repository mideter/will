#pragma once

#include "message_store_repository_impl.h"
#include "session_participant_notifier_impl.h"

#include "usecases/fetch_chat_history.h"
#include "usecases/send_chat_message.h"

#include <vector>


namespace will {


class MessageStore;
class Session;
class SessionRegistry;


/** Application coordinator: maps wire/session context to domain use cases. */
class ChatService {
public:
    ChatService(MessageStore& message_store, SessionRegistry& registry);

    void handle_user_chat(Session& sender, const std::vector<char>& payload);
    void handle_history_request(Session& sender, const std::vector<char>& payload);

private:
    MessageStoreMessageRepositoryImpl message_repository_;
    SessionParticipantNotifierImpl participant_notifier_;
    domain::SendChatMessage send_chat_message_;
    domain::FetchChatHistory fetch_chat_history_;
};


} // namespace will
