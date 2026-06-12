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
    if (adapter_.account_store_.has(connection_id_)) {
        on_bound_message(message);
        return;
    }

    if (adapter_.otp_state_.phase(connection_id_) == OtpConnectionPhase::AwaitingCode) {
        on_awaiting_code_message(message);
        return;
    }

    on_unbound_message(message);
}


void InboundClientMessageHandler::on_bound_message(const ClientMessage& message)
{
    if (dynamic_cast<const OtpPhoneRequestMessage*>(&message) != nullptr
        || dynamic_cast<const OtpCodeSubmitMessage*>(&message) != nullptr) {
        adapter_.close_with_protocol_error(connection_id_, "Protocol error: OTP frame while bound");
        return;
    }

    if (const auto* chat = dynamic_cast<const UserChatMessage*>(&message)) {
        adapter_.handle_user_chat(connection_id_, *chat);
        return;
    }

    if (const auto* history = dynamic_cast<const HistoryRequestMessage*>(&message)) {
        adapter_.handle_history_request(connection_id_, *history);
        return;
    }

    if (const auto* token = dynamic_cast<const BindTokenMessage*>(&message)) {
        adapter_.handle_bind_token(connection_id_, *token);
        return;
    }

    adapter_.close_with_protocol_error(connection_id_, "Protocol error: unhandled client message type");
}


void InboundClientMessageHandler::on_awaiting_code_message(const ClientMessage& message)
{
    if (const auto* code = dynamic_cast<const OtpCodeSubmitMessage*>(&message)) {
        adapter_.handle_otp_code_submit(connection_id_, *code);
        return;
    }

    adapter_.close_with_protocol_error(connection_id_, "Protocol error: expected OtpCodeSubmit");
}


void InboundClientMessageHandler::on_unbound_message(const ClientMessage& message)
{
    if (const auto* otp_request = dynamic_cast<const OtpPhoneRequestMessage*>(&message)) {
        adapter_.handle_otp_phone_request(connection_id_, *otp_request);
        return;
    }

    if (const auto* token = dynamic_cast<const BindTokenMessage*>(&message)) {
        adapter_.handle_bind_token(connection_id_, *token);
        return;
    }

    if (const auto* code = dynamic_cast<const OtpCodeSubmitMessage*>(&message)) {
        adapter_.close_with_protocol_error(connection_id_, "Protocol error: OtpCodeSubmit without OtpPhoneRequest");
        return;
    }

    if (dynamic_cast<const UserChatMessage*>(&message) != nullptr
        || dynamic_cast<const HistoryRequestMessage*>(&message) != nullptr) {
        adapter_.send_auth_required(connection_id_);
        return;
    }

    adapter_.close_with_protocol_error(connection_id_, "Protocol error: unhandled client message type");
}


} // namespace will
