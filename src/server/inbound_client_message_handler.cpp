#include "inbound_client_message_handler.h"

#include "protocoladapter.h"
#include "wiremessage_client.h"
#include "wiremessage_user_chat.h"


namespace will {


InboundClientMessageHandler::InboundClientMessageHandler(ProtocolAdapter& adapter,
                                                         const std::uint64_t connection_id)
    : adapter_(adapter)
    , connection_id_(connection_id)
{}


void InboundClientMessageHandler::on(const ClientMessage& message)
{
    if (const auto* login = dynamic_cast<const LoginRequestMessage*>(&message)) {
        adapter_.handle_login(connection_id_, *login);
        return;
    }

    if (const auto* token = dynamic_cast<const BindTokenMessage*>(&message)) {
        adapter_.handle_bind_token(connection_id_, *token);
        return;
    }

    if (const auto* chat = dynamic_cast<const UserChatMessage*>(&message)) {
        if (!adapter_.account_store_.has(connection_id_)) {
            adapter_.send_auth_required(connection_id_);
            return;
        }
        adapter_.handle_user_chat(connection_id_, *chat);
        return;
    }

    if (const auto* history = dynamic_cast<const HistoryRequestMessage*>(&message)) {
        if (!adapter_.account_store_.has(connection_id_)) {
            adapter_.send_auth_required(connection_id_);
            return;
        }
        adapter_.handle_history_request(connection_id_, *history);
        return;
    }

    adapter_.close_with_protocol_error(connection_id_, "Protocol error: unhandled client message type");
}


} // namespace will
