#include "session_participant_notifier_impl.h"

#include "sessionregistry.h"
#include "willprotocoladapter.h"


namespace will {


SessionParticipantNotifierImpl::SessionParticipantNotifierImpl(SessionRegistry& registry)
    : registry_(registry)
{}


void SessionParticipantNotifierImpl::notify_chat_message(const domain::ChatId chat,
                                                         const domain::Message& msg,
                                                         const domain::ParticipantId except_participant)
{
    (void)chat;
    const std::vector<char> payload = WillProtocolAdapter::encode_user_chat(msg.body);
    registry_.broadcast_except_participant(except_participant, payload);
}


void SessionParticipantNotifierImpl::send_to_participant(const domain::ParticipantId id,
                                                         const domain::OutboundEvent& ev)
{
    (void)id;
    (void)ev;
}


} // namespace will
