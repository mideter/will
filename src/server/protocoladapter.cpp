#include "protocoladapter.h"

#include "inbound_client_message_handler.h"

#include "values/device_token.h"
#include "values/timestamp.h"

#include <iostream>
#include <vector>


namespace will {


ProtocolAdapter::ProtocolAdapter(domain::World& world, SessionRegistry& registry)
	: world_(world)
	, registry_(registry)
	, participant_notifier_(registry, world_)
{
	world_.abode().echo_through(participant_notifier_);
}


void ProtocolAdapter::on_client_event(const SessionId session_id, const v1::ClientEvent& event)
{
	InboundClientMessageHandler handler{*this, session_id};
	handler.on(event);
}


bool ProtocolAdapter::is_authenticated(const SessionId session_id) const
{
	return registry_.is_authenticated(session_id);
}


void ProtocolAdapter::send_event(const SessionId session_id, const v1::ServerEvent& event)
{
	registry_.enqueue_event(session_id, event);
}


void ProtocolAdapter::close_with_protocol_error(const SessionId session_id, const std::string_view message)
{
	if (const std::string_view peer_address = registry_.peer_address(session_id); !peer_address.empty())
		std::cerr << "Session " << peer_address << ": " << message << '\n';
	else
		std::cerr << "Session " << session_id.value << ": " << message << '\n';

	close_session(session_id);
}


void ProtocolAdapter::close_session(const SessionId session_id)
{
	registry_.close_session(session_id);
}


void ProtocolAdapter::handle_bind_token(const SessionId session_id, const v1::BindToken& token)
{
	const auto device_token = domain::DeviceToken::parse(token.token());
	if (!device_token) {
		send_auth_required(session_id);
		return;
	}

	const domain::Man man = world_.welcome(*device_token);
	if (const auto displaced = registry_.bind_soul(session_id, man.soul_id()))
		close_session(*displaced);

	v1::ServerEvent event;
	event.mutable_auth_ok();
	send_event(session_id, event);
}


void ProtocolAdapter::send_auth_required(const SessionId session_id)
{
	v1::ServerEvent event;
	event.mutable_auth_required();
	send_event(session_id, event);
}


void ProtocolAdapter::handle_user_chat(const SessionId session_id, const v1::ChatMessage& chat)
{
	(void)world_.abode().inscribe(*registry_.soul_id(session_id), chat.body(), domain::Timestamp{});

	v1::ServerEvent event;
	event.mutable_receipt_ack();
	send_event(session_id, event);
}


void ProtocolAdapter::handle_history_request(const SessionId session_id, const v1::HistoryRequest& request)
{
	const auto outcome = world_.abode().retell(*registry_.soul_id(session_id), request.limit());
	if (const auto* error = std::get_if<domain::DomainError>(&outcome)) {
		(void)error;
		close_with_protocol_error(session_id, "Protocol error: invalid HistoryRequest");
		return;
	}

	const auto& items = std::get<std::vector<domain::RetoldLetter>>(outcome);
	for (const domain::RetoldLetter& item : items) {
		v1::ServerEvent event;
		auto* history_item = event.mutable_history_item();
		history_item->set_message_id(item.letter.id().value());
		history_item->set_is_mine(item.is_mine);
		history_item->set_name(item.author_name);
		history_item->set_body(item.letter.body());
		send_event(session_id, event);
	}

	v1::ServerEvent end_event;
	end_event.mutable_history_end();
	send_event(session_id, end_event);
}


} // namespace will
