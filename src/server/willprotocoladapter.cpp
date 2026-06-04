#include "willprotocoladapter.h"

#include "session.h"

#include <chrono>
#include <string_view>
#include <variant>

#include "support/anonymous_identity.h"
#include "willmessage.h"


namespace will {


WillProtocolAdapter::WillProtocolAdapter(domain::MessageRepository& message_repository,
                                         SessionRegistry& registry)
    : message_repository_(message_repository)
    , participant_notifier_(registry)
    , send_chat_message_(message_repository_, participant_notifier_)
    , fetch_chat_history_(message_repository_)
{}


void WillProtocolAdapter::on_client_frame(Session& session, const std::vector<char>& payload)
{
    if (!WillMessage::is_valid_client_to_server_payload(payload)) {
        session.fail_protocol("Protocol error: invalid client frame");
        return;
    }

    if (WillMessage::is_user_chat(payload)) {
        handle_user_chat(session, payload);
        return;
    }

    if (WillMessage::is_history_request(payload)) {
        handle_history_request(session, payload);
    }
}


void WillProtocolAdapter::handle_user_chat(Session& sender, const std::vector<char>& payload)
{
    const std::string body(payload.begin() + 1, payload.end());
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    const domain::Account account =
        domain::anonymous_account_for_peer(sender.peer_ip(), now_ms);

    const domain::SendChatMessageInput input{
        account,
        domain::ParticipantId{sender.id()},
        domain::ChatId::global(),
        body,
        now_ms,
    };

    (void)send_chat_message_.execute(input);
    sender.send_will_payload(WillMessage::encode_server_receipt_ack());
}


void WillProtocolAdapter::handle_history_request(Session& sender, const std::vector<char>& payload)
{
    const auto limit = WillMessage::parse_history_request_limit(payload);
    if (!limit) {
        sender.fail_protocol("Protocol error: invalid HistoryRequest");
        return;
    }

    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    const domain::Account account =
        domain::anonymous_account_for_peer(sender.peer_ip(), now_ms);

    const domain::FetchChatHistoryInput input{account, domain::ChatId::global(), *limit};

    const auto outcome = fetch_chat_history_.execute(input);
    if (const auto* error = std::get_if<domain::DomainError>(&outcome)) {
        (void)error;
        sender.fail_protocol("Protocol error: invalid HistoryRequest");
        return;
    }

    const auto& history = std::get<domain::FetchChatHistoryResult>(outcome);
    for (const domain::FetchChatHistoryItem& item : history.items) {
        sender.send_will_payload(WillMessage::encode_history_item(item.message.id, item.is_mine,
                                                                  item.message.body));
    }

    sender.send_will_payload(WillMessage::encode_history_end());
}


std::vector<char> WillProtocolAdapter::encode_user_chat(const std::string_view utf8_body)
{
    return WillMessage::encode_user_chat(utf8_body);
}


std::string WillProtocolAdapter::format_payload_for_log(const std::vector<char>& payload)
{
    return WillMessage::format_payload_for_log(payload);
}


} // namespace will
