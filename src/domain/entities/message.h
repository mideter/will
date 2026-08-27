#pragma once

#include "chat_id.h"
#include "message_id.h"
#include "timestamp.h"
#include "user_id.h"

#include <cstddef>
#include <string>


namespace will::domain {


/// Persisted chat message. Identity and body are fixed after construction.
class Message {
public:
    static constexpr std::size_t MaxBodyLength = 4096;

    /// Throws std::invalid_argument if author is zero, or body is empty / too long.
    Message(MessageId id, ChatId chat_id, UserId author_id, std::string body, Timestamp created_at);

    MessageId id() const noexcept { return id_; }
    ChatId chat_id() const noexcept { return chat_id_; }
    UserId author_id() const noexcept { return author_id_; }
    const std::string& body() const noexcept { return body_; }
    Timestamp created_at() const noexcept { return created_at_; }

private:
    MessageId id_;
    ChatId chat_id_;
    UserId author_id_;
    std::string body_;
    Timestamp created_at_;
};


} // namespace will::domain
