#pragma once

#include "connectionaccountstore.h"
#include "tcpconnectionparticipantnotifierimpl.h"

#include "ports/messenger_persistence.h"

#include "usecases/authenticate_user.h"
#include "usecases/fetch_chat_history.h"
#include "usecases/send_chat_message.h"

#include "wiremessage.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>


namespace will {


class TcpConnectionRegistry;


struct InboundClientFrameHandler;


/** Maps wire payloads to domain use cases and encodes outbound frames. */
class ProtocolAdapter {
    friend struct InboundClientFrameHandler;

public:
    ProtocolAdapter(domain::MessengerPersistence persistence, TcpConnectionRegistry& registry,
                    ConnectionAccountStore& account_store);

    void on_client_frame(std::uint64_t connection_id, const std::vector<char>& payload);

    static std::vector<char> encode_user_chat(std::string_view utf8_body);
    static std::string format_payload_for_log(const std::vector<char>& payload);

private:
    void handle_login(std::uint64_t connection_id, const LoginRequestMessage& request);
    void handle_bind_token(std::uint64_t connection_id, const BindTokenMessage& token);
    void handle_user_chat(std::uint64_t connection_id, const UserChatMessage& chat);
    void handle_history_request(std::uint64_t connection_id, const HistoryRequestMessage& request);
    void send_auth_required(std::uint64_t connection_id);
    void send_payload(std::uint64_t connection_id, const std::vector<char>& app_payload);
    void close_with_protocol_error(std::uint64_t connection_id, std::string_view message);

    domain::MessengerPersistence persistence_;
    TcpConnectionRegistry& registry_;
    ConnectionAccountStore& account_store_;
    TcpConnectionParticipantNotifierImpl participant_notifier_;
    domain::AuthenticateUser authenticate_user_;
    domain::SendChatMessage send_chat_message_;
    domain::FetchChatHistory fetch_chat_history_;
};


} // namespace will
