#pragma once

#include "ports/message_repository.h"

#include <string_view>


namespace will {


class MessageStore;


class MessageStoreMessageRepositoryImpl final : public domain::MessageRepository {
public:
    explicit MessageStoreMessageRepositoryImpl(MessageStore& store);

    void bind_sender(std::string_view peer_address) noexcept;

    domain::Message append(domain::ChatId chat, domain::UserId author, std::string_view body,
                           domain::TimestampMs ts) override;

    std::vector<domain::Message> load_last(domain::ChatId chat, std::uint32_t limit) override;

private:
    MessageStore& store_;
    std::string_view sender_address_;
};


} // namespace will
