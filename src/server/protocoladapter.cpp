#include "protocoladapter.h"

#include "inbound_client_message_handler.h"
#include "tcpconnectionregistry.h"

#include "support/phone_number.h"

#include <chrono>
#include <iostream>
#include <string_view>

#include "tcpframe.h"
#include "wiremessage_codec.h"
#include "wiremessage_server.h"


namespace will {


namespace {


std::uint8_t otp_error_code_for(const domain::OtpError error)
{
    switch (error) {
    case domain::OtpError::InvalidPhone:
        return static_cast<std::uint8_t>(OtpVerifyResponseMessage::Error::InvalidPhone);
    case domain::OtpError::RateLimited:
        return static_cast<std::uint8_t>(OtpVerifyResponseMessage::Error::RateLimited);
    case domain::OtpError::InvalidCode:
        return static_cast<std::uint8_t>(OtpVerifyResponseMessage::Error::InvalidCode);
    case domain::OtpError::Expired:
        return static_cast<std::uint8_t>(OtpVerifyResponseMessage::Error::Expired);
    case domain::OtpError::Internal:
    default:
        return static_cast<std::uint8_t>(OtpVerifyResponseMessage::Error::Internal);
    }
}


domain::RequestOtpConfig request_otp_config_from(const ServerConfig& config)
{
    domain::RequestOtpConfig otp_config;
    otp_config.otp_ttl_sec = config.otp_ttl_sec;
    otp_config.otp_length = config.otp_length;
    otp_config.otp_cooldown_sec = config.otp_cooldown_sec;
    otp_config.max_requests_per_ip = config.max_requests_per_ip;
    otp_config.ip_rate_window_sec = config.ip_rate_window_sec;
    otp_config.hash_salt = config.otp_hash_salt;
    otp_config.dev_fixed_otp = config.dev_fixed_otp;
    return otp_config;
}


domain::VerifyOtpConfig verify_otp_config_from(const ServerConfig& config)
{
    domain::VerifyOtpConfig otp_config;
    otp_config.max_verify_attempts = config.max_verify_attempts;
    otp_config.hash_salt = config.otp_hash_salt;
    return otp_config;
}


} // namespace


ProtocolAdapter::ProtocolAdapter(const ServerConfig& config, domain::MessengerPersistence persistence,
                                 domain::OtpStore& otp_store, domain::SmsSender& sms_sender,
                                 domain::OtpHasher& otp_hasher, TcpConnectionRegistry& registry,
                                 ConnectionAccountStore& account_store, OtpConnectionStateStore& otp_state)
    : persistence_(persistence)
    , registry_(registry)
    , account_store_(account_store)
    , otp_state_(otp_state)
    , participant_notifier_(registry)
    , request_otp_(otp_store, sms_sender, otp_hasher, request_otp_config_from(config))
    , verify_otp_(persistence.users, otp_store, persistence.sessions, otp_hasher, verify_otp_config_from(config))
    , send_chat_message_(persistence.messages, participant_notifier_)
    , fetch_chat_history_(persistence.messages)
{}


void ProtocolAdapter::on_client_payload(const std::uint64_t connection_id, const std::vector<char>& payload)
{
    const auto message = WireMessageCodec::decode_client(payload);
    if (!message) {
        close_with_protocol_error(connection_id, "Protocol error: invalid client frame");
        return;
    }

    InboundClientMessageHandler handler{*this, connection_id};
    handler.on(*message);
}


void ProtocolAdapter::send_payload(const std::uint64_t connection_id, const std::vector<char>& payload)
{
    registry_.enqueue_wire_frame(connection_id, TcpFrame::encode(payload));
}


void ProtocolAdapter::close_with_protocol_error(const std::uint64_t connection_id, const std::string_view message)
{
    if (const std::string_view peer_address = registry_.peer_address(connection_id); !peer_address.empty())
        std::cerr << "Connection " << peer_address << ": " << message << '\n';
    else
        std::cerr << "Connection " << connection_id << ": " << message << '\n';

    close_connection(connection_id);
}


void ProtocolAdapter::close_connection(const std::uint64_t connection_id)
{
    otp_state_.clear(connection_id);
    registry_.close_connection(connection_id);
}


void ProtocolAdapter::handle_otp_phone_request(const std::uint64_t connection_id,
                                               const OtpPhoneRequestMessage& request)
{
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    const std::string_view peer_ip = registry_.peer_address(connection_id);
    const domain::RequestOtpInput input{request.phone_e164(), peer_ip, now_ms};
    const auto outcome = request_otp_.execute(input);

    if (const auto* failure = std::get_if<domain::OtpError>(&outcome)) {
        send_payload(connection_id, WireMessageCodec::encode(OtpVerifyResponseMessage{
                                       false, "", otp_error_code_for(*failure)}));
        close_connection(connection_id);
        return;
    }

    send_payload(connection_id, WireMessageCodec::encode(OtpSentMessage{}));
    otp_state_.begin_awaiting_code(connection_id, request.phone_e164(),
                                   [this](const std::uint64_t id) {
                                       send_payload(id, WireMessageCodec::encode(OtpVerifyResponseMessage{
                                                           false, "",
                                                           static_cast<std::uint8_t>(
                                                               OtpVerifyResponseMessage::Error::Expired)}));
                                       close_connection(id);
                                   });
}


void ProtocolAdapter::handle_otp_code_submit(const std::uint64_t connection_id,
                                             const OtpCodeSubmitMessage& request)
{
    const std::optional<std::string> pending_phone = otp_state_.pending_phone(connection_id);
    if (!pending_phone) {
        close_with_protocol_error(connection_id, "Protocol error: OTP code submit without pending phone");
        return;
    }

    const auto phone = domain::PhoneNumber::parse(*pending_phone);
    if (!phone) {
        close_with_protocol_error(connection_id, "Protocol error: invalid pending phone");
        return;
    }

    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    const domain::VerifyOtpInput input{*phone, request.code(), now_ms};
    const auto outcome = verify_otp_.execute(input);

    if (const auto* failure = std::get_if<domain::OtpError>(&outcome)) {
        send_payload(connection_id, WireMessageCodec::encode(OtpVerifyResponseMessage{
                                       false, "", otp_error_code_for(*failure)}));
        close_connection(connection_id);
        return;
    }

    const auto& success = std::get<domain::VerifyOtpSuccess>(outcome);
    otp_state_.clear(connection_id);
    send_payload(connection_id,
                 WireMessageCodec::encode(OtpVerifyResponseMessage{true, success.token.value, 0}));
}


void ProtocolAdapter::handle_bind_token(const std::uint64_t connection_id, const BindTokenMessage& token)
{
    const auto account = persistence_.sessions.resolve_token(domain::AuthToken{token.token()});
    if (!account) {
        send_auth_required(connection_id);
        return;
    }

    account_store_.set(connection_id, *account);
}


void ProtocolAdapter::send_auth_required(const std::uint64_t connection_id)
{
    send_payload(connection_id, WireMessageCodec::encode(AuthRequiredMessage{}));
}


void ProtocolAdapter::handle_user_chat(const std::uint64_t connection_id, const UserChatMessage& chat)
{
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    const domain::SendChatMessageInput input{
        *account_store_.get(connection_id),
        domain::ParticipantId{connection_id},
        domain::ChatId::global(),
        chat.body(),
        now_ms,
    };

    (void)send_chat_message_.execute(input);
    send_payload(connection_id, WireMessageCodec::encode(ServerReceiptAckMessage{}));
}


void ProtocolAdapter::handle_history_request(const std::uint64_t connection_id,
                                             const HistoryRequestMessage& request)
{
    const domain::FetchChatHistoryInput input{*account_store_.get(connection_id), domain::ChatId::global(),
                                              request.limit()};

    const auto outcome = fetch_chat_history_.execute(input);
    if (const auto* error = std::get_if<domain::DomainError>(&outcome)) {
        (void)error;
        close_with_protocol_error(connection_id, "Protocol error: invalid HistoryRequest");
        return;
    }

    const auto& history = std::get<domain::FetchChatHistoryResult>(outcome);
    for (const domain::FetchChatHistoryItem& item : history.items) {
        send_payload(connection_id, WireMessageCodec::encode(HistoryItemMessage{item.message.id, item.is_mine, item.message.body}));
    }

    send_payload(connection_id, WireMessageCodec::encode(HistoryEndMessage{}));
}


} // namespace will
