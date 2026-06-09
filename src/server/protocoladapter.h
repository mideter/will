#pragma once

#include "tcpconnectionparticipantnotifierimpl.h"

#include "ports/messenger_persistence.h"

#include "usecases/authenticate_user.h"
#include "usecases/fetch_chat_history.h"
#include "usecases/send_chat_message.h"

#include <string>
#include <string_view>
#include <vector>


namespace will {


class TcpConnection;
class TcpConnectionRegistry;


/** Maps wire payloads to domain use cases and encodes outbound frames. */
class ProtocolAdapter {
public:
    ProtocolAdapter(domain::MessengerPersistence persistence, TcpConnectionRegistry& registry);

    void on_client_frame(TcpConnection& connection, const std::vector<char>& payload);

    static std::vector<char> encode_user_chat(std::string_view utf8_body);
    static std::string format_payload_for_log(const std::vector<char>& payload);

private:
    void handle_login(TcpConnection& connection, const std::vector<char>& payload);
    void handle_bind_token(TcpConnection& connection, const std::vector<char>& payload);
    void handle_user_chat(TcpConnection& sender, const std::vector<char>& payload);
    void handle_history_request(TcpConnection& sender, const std::vector<char>& payload);
    void send_auth_required(TcpConnection& connection);

    domain::MessengerPersistence persistence_;
    TcpConnectionParticipantNotifierImpl participant_notifier_;
    domain::AuthenticateUser authenticate_user_;
    domain::SendChatMessage send_chat_message_;
    domain::FetchChatHistory fetch_chat_history_;
};


} // namespace will
