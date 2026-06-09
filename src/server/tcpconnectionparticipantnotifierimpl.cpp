#include "tcpconnectionparticipantnotifierimpl.h"

#include "tcpconnectionregistry.h"
#include "willprotocoladapter.h"


namespace will {


TcpConnectionParticipantNotifierImpl::TcpConnectionParticipantNotifierImpl(TcpConnectionRegistry& registry)
    : registry_(registry)
{}


void TcpConnectionParticipantNotifierImpl::notify_chat_message(const domain::ChatId chat,
                                                                 const domain::Message& msg,
                                                                 const domain::ParticipantId except_participant)
{
    (void)chat;
    const std::vector<char> payload = WillProtocolAdapter::encode_user_chat(msg.body);
    registry_.broadcast_except_participant(except_participant, payload);
}


void TcpConnectionParticipantNotifierImpl::send_to_participant(const domain::ParticipantId id,
                                                               const domain::OutboundEvent& ev)
{
    (void)id;
    (void)ev;
}


} // namespace will
