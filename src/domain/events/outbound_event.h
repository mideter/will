#pragma once

#include "entities/message.h"

#include <variant>


namespace will::domain {


struct OutboundChatMessage {
    Message message;
};


/// Targeted notification payload for a single participant (adapter maps to wire).
using OutboundEvent = std::variant<OutboundChatMessage>;


} // namespace will::domain
