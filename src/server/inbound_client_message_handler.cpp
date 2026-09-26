#include "inbound_client_message_handler.h"

#include "protocoladapter.h"


namespace will {


InboundClientMessageHandler::InboundClientMessageHandler(ProtocolAdapter& adapter, const SessionId session_id)
	: adapter_(adapter)
	, session_id_(session_id)
{}


void InboundClientMessageHandler::on(const v1::ClientEvent& event)
{
	if (adapter_.is_authenticated(session_id_)) {
		on_bound_event(event);
		return;
	}

	on_unbound_event(event);
}


void InboundClientMessageHandler::on_bound_event(const v1::ClientEvent& event)
{
	switch (event.event_case()) {
	case v1::ClientEvent::kChat:
		adapter_.handle_user_chat(session_id_, event.chat());
		return;
	case v1::ClientEvent::kHistoryRequest:
		adapter_.handle_history_request(session_id_, event.history_request());
		return;
	case v1::ClientEvent::kBindToken:
		adapter_.handle_bind_token(session_id_, event.bind_token());
		return;
	case v1::ClientEvent::kSupplicate:
		adapter_.handle_supplicate(session_id_, event.supplicate());
		return;
	case v1::ClientEvent::kAcceptSupplication:
		adapter_.handle_accept_supplication(session_id_, event.accept_supplication());
		return;
	case v1::ClientEvent::kWillDeed:
		adapter_.handle_will_deed(session_id_, event.will_deed());
		return;
	case v1::ClientEvent::kExecuteDeed:
		adapter_.handle_execute_deed(session_id_, event.execute_deed());
		return;
	case v1::ClientEvent::EVENT_NOT_SET:
		break;
	}

	adapter_.close_with_protocol_error(session_id_, "Protocol error: unhandled client message type");
}


void InboundClientMessageHandler::on_unbound_event(const v1::ClientEvent& event)
{
	switch (event.event_case()) {
	case v1::ClientEvent::kBindToken:
		adapter_.handle_bind_token(session_id_, event.bind_token());
		return;
	case v1::ClientEvent::kChat:
	case v1::ClientEvent::kHistoryRequest:
	case v1::ClientEvent::kSupplicate:
	case v1::ClientEvent::kAcceptSupplication:
	case v1::ClientEvent::kWillDeed:
	case v1::ClientEvent::kExecuteDeed:
		adapter_.send_auth_required(session_id_);
		return;
	case v1::ClientEvent::EVENT_NOT_SET:
		break;
	}

	adapter_.close_with_protocol_error(session_id_, "Protocol error: unhandled client message type");
}


} // namespace will
