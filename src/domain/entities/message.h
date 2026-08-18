#pragma once

// Declarations shared with module will.domain.message (message.cppm).
// Included from use-case / port headers. In new .cpp files prefer:
//   import will.domain.message;

#include "chat_id.h"
#include "timestamp.h"
#include "user_id.h"

#include <cstdint>
#include <string>


namespace will::domain {


struct Message {
    std::uint64_t id = 0;
    ChatId chat_id = ChatId::global();
    UserId author_id{};
    std::string body;
    Timestamp created_at{};
    std::string author_name; // filled at send/load time; not a messages column
};


} // namespace will::domain
