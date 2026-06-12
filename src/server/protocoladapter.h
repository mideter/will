#pragma once

#include "connectionaccountstore.h"
#include "otpconnectionstatestore.h"
#include "serverconfig.h"
#include "tcpconnectionparticipantnotifierimpl.h"

#include "ports/messenger_persistence.h"
#include "ports/otp_hasher.h"
#include "ports/otp_store.h"
#include "ports/sms_sender.h"

#include "usecases/fetch_chat_history.h"
#include "usecases/request_otp.h"
#include "usecases/send_chat_message.h"
#include "usecases/verify_otp.h"

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
    ProtocolAdapter(const ServerConfig& config, domain::MessengerPersistence persistence, domain::OtpStore& otp_store,
                    domain::SmsSender& sms_sender, domain::OtpHasher& otp_hasher, TcpConnectionRegistry& registry,
                    ConnectionAccountStore& account_store, OtpConnectionStateStore& otp_state);

    void on_client_payload(std::uint64_t connection_id, const std::vector<char>& payload);

private:
    void handle_otp_phone_request(std::uint64_t connection_id, const OtpPhoneRequestMessage& request);
    void handle_otp_code_submit(std::uint64_t connection_id, const OtpCodeSubmitMessage& request);
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
    OtpConnectionStateStore& otp_state_;
    TcpConnectionParticipantNotifierImpl participant_notifier_;
    domain::RequestOtp request_otp_;
    domain::VerifyOtp verify_otp_;
    domain::SendChatMessage send_chat_message_;
    domain::FetchChatHistory fetch_chat_history_;
};


} // namespace will
