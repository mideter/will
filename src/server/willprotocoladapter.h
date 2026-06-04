#pragma once

#include "session_participant_notifier_impl.h"

#include "ports/message_repository.h"

#include "usecases/fetch_chat_history.h"
#include "usecases/send_chat_message.h"

#include <string>
#include <string_view>
#include <vector>


namespace will {


class Session;
class SessionRegistry;


/** Maps Will wire payloads to domain use cases and encodes outbound frames. */
class WillProtocolAdapter {
public:
    WillProtocolAdapter(domain::MessageRepository& message_repository, SessionRegistry& registry);

    void on_client_frame(Session& session, const std::vector<char>& payload);

    static std::vector<char> encode_user_chat(std::string_view utf8_body);
    static std::string format_payload_for_log(const std::vector<char>& payload);

private:
    void handle_user_chat(Session& sender, const std::vector<char>& payload);
    void handle_history_request(Session& sender, const std::vector<char>& payload);

    domain::MessageRepository& message_repository_;
    SessionParticipantNotifierImpl participant_notifier_;
    domain::SendChatMessage send_chat_message_;
    domain::FetchChatHistory fetch_chat_history_;
};


} // namespace will
