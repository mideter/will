#include "chatservice.h"

#include "session.h"

#include <chrono>
#include <variant>

#include "support/anonymous_identity.h"
#include "willmessage.h"


namespace will {


ChatService::ChatService(MessageStore& message_store, SessionRegistry& registry)
    : message_repository_(message_store)
    , participant_notifier_(registry)
    , send_chat_message_(message_repository_, participant_notifier_)
    , fetch_chat_history_(message_repository_)
{}


void ChatService::handle_user_chat(Session& sender, const std::vector<char>& payload)
{
    const std::string body(payload.begin() + 1, payload.end());
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    message_repository_.bind_sender(sender.peer_ip());

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


void ChatService::handle_history_request(Session& sender, const std::vector<char>& payload)
{
    const auto limit = WillMessage::parse_history_request_limit(payload);
    if (!limit) {
        sender.fail_protocol("Protocol error: invalid HistoryRequest");
        return;
    }

    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    message_repository_.bind_sender(sender.peer_ip());

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


} // namespace will
