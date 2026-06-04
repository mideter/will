#include "message_store_repository_impl.h"

#include "messagestore.h"

#include "support/anonymous_identity.h"


namespace will {


MessageStoreMessageRepositoryImpl::MessageStoreMessageRepositoryImpl(MessageStore& store)
    : store_(store)
{}


void MessageStoreMessageRepositoryImpl::bind_sender(const std::string_view peer_address) noexcept
{
    sender_address_ = peer_address;
}


domain::Message MessageStoreMessageRepositoryImpl::append(const domain::ChatId chat,
                                                          const domain::UserId author,
                                                          const std::string_view body,
                                                          const domain::TimestampMs ts)
{
    (void)author;
    const std::uint64_t id = store_.insert_message(body, sender_address_, ts);
    return domain::Message{id, chat, domain::user_id_for_peer_address(sender_address_),
                           std::string(body), ts};
}


std::vector<domain::Message> MessageStoreMessageRepositoryImpl::load_last(const domain::ChatId chat,
                                                                          const std::uint32_t limit)
{
    const std::vector<StoredMessage> rows = store_.load_last(limit);
    std::vector<domain::Message> messages;
    messages.reserve(rows.size());

    for (const StoredMessage& row : rows) {
        messages.push_back(domain::Message{
            row.id, chat, domain::user_id_for_peer_address(row.sender_ip), row.body, row.created_at_ms});
    }

    return messages;
}


} // namespace will
