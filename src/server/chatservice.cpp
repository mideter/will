#include "chatservice.h"

#include "messagestore.h"
#include "session.h"
#include "sessionregistry.h"

#include <algorithm>
#include <chrono>

#include "willmessage.h"
#include "willprotocol.h"


namespace will {


ChatService::ChatService(MessageStore& message_store, SessionRegistry& registry)
    : message_store_(message_store)
    , registry_(registry)
{}


void ChatService::handle_user_chat(Session& sender, const std::vector<char>& payload)
{
    const std::string body(payload.begin() + 1, payload.end());
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
    message_store_.insert_message(body, sender.peer_ip(), now_ms);

    sender.send_will_payload(WillMessage::encode_server_receipt_ack());
    registry_.broadcast_except(sender, payload);
}


void ChatService::handle_history_request(Session& sender, const std::vector<char>& payload)
{
    const auto limit = WillMessage::parse_history_request_limit(payload);
    if (!limit) {
        sender.fail_protocol("Protocol error: invalid HistoryRequest");
        return;
    }

    const std::uint32_t capped_limit = std::min(*limit, WillMessage::MaxHistoryRequestLimit);
    const auto rows = message_store_.load_last(capped_limit);
    const std::string_view viewer_ip = sender.peer_ip();

    for (const StoredMessage& row : rows) {
        const bool is_mine = row.sender_ip == viewer_ip;
        sender.send_will_payload(
            WillMessage::encode_history_item(row.id, is_mine, row.body));
    }

    sender.send_will_payload(WillMessage::encode_history_end());
}


} // namespace will
