#pragma once

#include "chat_id.h"
#include "user_id.h"

#include <cstdint>
#include <string>


namespace will::domain {


using TimestampMs = std::int64_t;


struct Message {
    std::uint64_t id = 0;
    ChatId chat_id = ChatId::global();
    UserId author_id{};
    std::string body;
    TimestampMs created_at = 0;
};


} // namespace will::domain
