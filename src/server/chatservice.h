#pragma once

#include <vector>


namespace will {


class MessageStore;
class Session;
class SessionRegistry;


/** Application logic for inbound Will chat and history messages. */
class ChatService {
public:
    ChatService(MessageStore& message_store, SessionRegistry& registry);

    void handle_user_chat(Session& sender, const std::vector<char>& payload);
    void handle_history_request(Session& sender, const std::vector<char>& payload);

private:
    MessageStore& message_store_;
    SessionRegistry& registry_;
};


} // namespace will
