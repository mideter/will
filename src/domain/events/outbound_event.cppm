export module will.domain.outbound_event;

export import will.domain.message;

export namespace will::domain {


struct OutboundChatMessage {
    Message message;
};


/// Targeted notification payload for a single participant (adapter maps to wire).
struct OutboundEvent {
    OutboundChatMessage chat;
};


} // namespace will::domain
