export module will.domain.participant_notifier;

export import will.domain.chat_id;
export import will.domain.message;
export import will.domain.outbound_event;
export import will.domain.participant_id;

export namespace will::domain {


class ParticipantNotifier {
public:
    virtual ~ParticipantNotifier() = default;

    virtual void notify_chat_message(ChatId chat, const Message& msg, ParticipantId except_participant) = 0;
    virtual void send_to_participant(ParticipantId id, const OutboundEvent& ev) = 0;
};


} // namespace will::domain
