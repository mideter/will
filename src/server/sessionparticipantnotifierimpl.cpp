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


void SessionParticipantNotifierImpl::notify_letter(const domain::Letter& letter)
{
    std::string author_name;

    if (const auto author = users_.find_by_id(letter.author_id()))
        author_name = author->name().text();

    v1::ServerEvent event;
    auto* chat_message = event.mutable_chat();
    chat_message->set_name(author_name);
    chat_message->set_body(letter.body());

    if (const auto session_id = registry_.session_id_for_user(letter.author_id())) {
        if (const std::string_view sender_address = registry_.peer_address(*session_id); !sender_address.empty()) {
            std::cout << "Broadcast from " << sender_address << ": chat name_len=" << author_name.size()
                      << " body_len=" << letter.body().size() << std::endl;
        }
    }

    registry_.broadcast_except_user(letter.author_id(), event);
}


} // namespace will
