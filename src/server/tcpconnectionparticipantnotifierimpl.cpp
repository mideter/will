#include "tcpconnectionparticipantnotifierimpl.h"

#include <iostream>

#include "tcpconnectionregistry.h"
#include "wiremessage_codec.h"
#include "wiremessage_user_chat.h"
#include "tcpframe.h"


namespace will {


TcpConnectionParticipantNotifierImpl::TcpConnectionParticipantNotifierImpl(TcpConnectionRegistry& registry)
    : registry_(registry)
{}


void TcpConnectionParticipantNotifierImpl::notify_chat_message(const domain::ChatId chat,
                                                                 const domain::Message& msg,
                                                                 const domain::ParticipantId except_participant)
{
    (void)chat;

    const std::vector<char> payload = WireMessageCodec::encode(UserChatMessage{msg.body});

    if (const std::string_view sender_address = registry_.peer_address(except_participant.value);
        !sender_address.empty()) {
        std::cout << "Broadcast from " << sender_address << ": " << WireMessageCodec::format_for_log(payload)
                  << std::endl;
    }

    registry_.broadcast_wire_except(except_participant.value, TcpFrame::encode(payload));
}


void TcpConnectionParticipantNotifierImpl::send_to_participant(const domain::ParticipantId id,
                                                               const domain::OutboundEvent& ev)
{
    (void)id;
    (void)ev;
}


} // namespace will
