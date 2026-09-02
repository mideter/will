#pragma once

#include "connectionuserstore.h"
#include "serverconfig.h"
#include "sessionparticipantnotifierimpl.h"

#include "ports/messenger_persistence.h"

#include "usecases/authenticate_device.h"
#include "usecases/fetch_chat_history.h"
#include "usecases/send_chat_message.h"

#include "infra/transport/messenger.pb.h"

#include <cstdint>
#include <string_view>


namespace will {


class SessionRegistry;


class InboundClientMessageHandler;


/** Maps client protobuf events to domain use cases and enqueues outbound events. */
class ProtocolAdapter {
    friend class InboundClientMessageHandler;

public:
    ProtocolAdapter(domain::MessengerPersistence persistence, SessionRegistry& registry,
                    ConnectionUserStore& user_store);

    void on_client_event(std::uint64_t session_id, const v1::ClientEvent& event);

private:
    void handle_bind_token(std::uint64_t session_id, const v1::BindToken& token);
    void handle_user_chat(std::uint64_t session_id, const v1::ChatMessage& chat);
    void handle_history_request(std::uint64_t session_id, const v1::HistoryRequest& request);
    void send_auth_required(std::uint64_t session_id);
    void send_event(std::uint64_t session_id, const v1::ServerEvent& event);
    void close_with_protocol_error(std::uint64_t session_id, std::string_view message);
    void close_session(std::uint64_t session_id);

    domain::MessengerPersistence persistence_;
    SessionRegistry& registry_;
    ConnectionUserStore& user_store_;
    SessionParticipantNotifierImpl participant_notifier_;
    domain::AuthenticateDevice authenticate_device_;
    domain::SendChatMessage send_chat_message_;
    domain::FetchChatHistory fetch_chat_history_;
};


} // namespace will
