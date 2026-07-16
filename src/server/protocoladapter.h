#pragma once

#include "connectionaccountstore.h"
#include "serverconfig.h"
#include "tcpconnectionparticipantnotifierimpl.h"

#include "ports/messenger_persistence.h"

#include "usecases/authenticate_device.h"
#include "usecases/fetch_chat_history.h"
#include "usecases/send_chat_message.h"

#include "wiremessage.h"
#include "wiremessage_client.h"
#include "wiremessage_user_chat.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>


namespace will {


class TcpConnectionRegistry;


class InboundClientMessageHandler;


/** Maps wire payloads to domain use cases and enqueues outbound wire frames. */
class ProtocolAdapter {
    friend class InboundClientMessageHandler;

public:
    ProtocolAdapter(domain::MessengerPersistence persistence, TcpConnectionRegistry& registry,
                    ConnectionAccountStore& account_store);

    void on_client_payload(std::uint64_t connection_id, const std::vector<char>& payload);

private:
    void handle_bind_token(std::uint64_t connection_id, const BindTokenMessage& token);
    void handle_user_chat(std::uint64_t connection_id, const UserChatMessage& chat);
    void handle_history_request(std::uint64_t connection_id, const HistoryRequestMessage& request);
    void send_auth_required(std::uint64_t connection_id);
    void send_payload(std::uint64_t connection_id, const std::vector<char>& payload);
    void close_with_protocol_error(std::uint64_t connection_id, std::string_view message);
    void close_connection(std::uint64_t connection_id);

    domain::MessengerPersistence persistence_;
    TcpConnectionRegistry& registry_;
    ConnectionAccountStore& account_store_;
    TcpConnectionParticipantNotifierImpl participant_notifier_;
    domain::AuthenticateDevice authenticate_device_;
    domain::SendChatMessage send_chat_message_;
    domain::FetchChatHistory fetch_chat_history_;
};


} // namespace will
