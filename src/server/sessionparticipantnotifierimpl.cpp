#include "sessionparticipantnotifierimpl.h"

#include "sessionregistry.h"

#include "infra/transport/messenger.pb.h"

#include <iostream>


namespace will {


SessionParticipantNotifierImpl::SessionParticipantNotifierImpl(SessionRegistry& registry,
                                                               domain::UserRepository& users)
    : registry_(registry)
    , users_(users)
{}


void SessionParticipantNotifierImpl::notify_chat_message(const domain::Message& msg,
                                                         const domain::ParticipantId except_participant)
{
    std::string author_name;
    
    if (const auto author = users_.find_by_id(msg.author_id()))
        author_name = author->name().text();

    v1::ServerEvent event;
    auto* chat_message = event.mutable_chat();
    chat_message->set_name(author_name);
    chat_message->set_body(msg.body());

    if (const std::string_view sender_address = registry_.peer_address(except_participant.value);
        !sender_address.empty()) {
        std::cout << "Broadcast from " << sender_address << ": chat name_len=" << author_name.size()
                  << " body_len=" << msg.body().size() << std::endl;
    }

    registry_.broadcast_except(except_participant.value, event);
}


} // namespace will
