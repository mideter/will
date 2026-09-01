#include "message.h"

#include <stdexcept>


namespace will::domain {


Message::Message(const MessageId id, const ChatId chat_id, const UserId author_id, std::string body,
                 const Timestamp created_at)
    : id_(id)
    , chat_id_(chat_id)
    , author_id_(author_id)
    , body_(std::move(body))
    , created_at_(created_at)
{
    if (body_.empty())
        throw std::invalid_argument("Message body must be non-empty");
    if (body_.size() > MaxBodyLength)
        throw std::invalid_argument("Message body exceeds MaxBodyLength");
}


} // namespace will::domain
