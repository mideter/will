#include "protocoladapter.h"

#include "inbound_client_message_handler.h"

#include "entities/witness.h"
#include "values/device_token.h"

#include <iostream>
#include <vector>


namespace will {


ProtocolAdapter::ProtocolAdapter(domain::World& world, SessionRegistry& registry)
	: world_(world)
	, registry_(registry)
{}


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

	const domain::Man& man = world_.welcome(*device_token);
	if (const auto displaced = registry_.bind_soul(session_id, man.Soul::id()))
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
	const auto soul_id = registry_.soul_id(session_id);
	if (!soul_id)
		return;

	const domain::Man& man = static_cast<const domain::Man&>(world_.soul(*soul_id));
	man.say(chat.body());

	std::string author_name{man.name().text()};

	v1::ServerEvent chat_event;
	auto* chat_message = chat_event.mutable_chat();
	chat_message->set_name(author_name);
	chat_message->set_body(chat.body());

	if (const std::string_view sender_address = registry_.peer_address(session_id); !sender_address.empty()) {
		std::cout << "Broadcast from " << sender_address << ": chat name_len=" << author_name.size()
				  << " body_len=" << chat.body().size() << std::endl;
	}

	registry_.broadcast_except_soul(man.Soul::id(), chat_event);

	v1::ServerEvent ack;
	ack.mutable_receipt_ack();
	send_event(session_id, ack);
}


void ProtocolAdapter::handle_history_request(const SessionId session_id, const v1::HistoryRequest& request)
{
	const auto soul_id = registry_.soul_id(session_id);
	if (!soul_id)
		return;

	const domain::Man& man = static_cast<const domain::Man&>(world_.soul(*soul_id));
	const auto& witness = static_cast<const domain::Witness&>(man);
	const auto outcome = witness.abode().retell(man.Soul::id(), request.limit());
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
