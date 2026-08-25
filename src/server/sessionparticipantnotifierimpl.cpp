#include "sessionparticipantnotifierimpl.h"

#include "sessionregistry.h"

#include "proto/messenger.pb.h"

#include <iostream>


namespace will {


SessionParticipantNotifierImpl::SessionParticipantNotifierImpl(SessionRegistry& registry)
    : registry_(registry)
{}


void SessionParticipantNotifierImpl::notify_chat_message(const domain::ChatId chat,
                                                         const domain::Message& msg,
                                                         const domain::ParticipantId except_participant)
{
    (void)chat;

    v1::ServerEvent event;
    auto* chat_message = event.mutable_chat();
    chat_message->set_name(msg.author_name);
    chat_message->set_body(msg.body);

    if (const std::string_view sender_address = registry_.peer_address(except_participant.value);
        !sender_address.empty()) {
        std::cout << "Broadcast from " << sender_address << ": chat name_len=" << msg.author_name.size()
                  << " body_len=" << msg.body.size() << std::endl;
    }

    registry_.broadcast_except(except_participant.value, event);
}


void SessionParticipantNotifierImpl::send_to_participant(const domain::ParticipantId id,
                                                         const domain::OutboundEvent& ev)
{
    (void)id;
    (void)ev;
}


} // namespace will
