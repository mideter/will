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
	// An unborn body only waits.
	if (event.event_case() != v1::ClientEvent::kBindToken && event.event_case() != v1::ClientEvent::EVENT_NOT_SET
		&& adapter_.awaits_birth(session_id_)) {
		adapter_.send_notice(session_id_, "one awaits one's birth");
		return;
	}

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
	case v1::ClientEvent::kTurn:
		adapter_.handle_turn(session_id_, event.turn());
		return;
	case v1::ClientEvent::kFulfil:
		adapter_.handle_fulfil(session_id_, event.fulfil());
		return;
	case v1::ClientEvent::kAdmit:
		adapter_.handle_admit(session_id_, event.admit());
		return;
	case v1::ClientEvent::kRegard:
		adapter_.handle_regard(session_id_, event.regard());
		return;
	case v1::ClientEvent::kListDwellers:
		adapter_.handle_list_dwellers(session_id_);
		return;
	case v1::ClientEvent::kArrange:
		adapter_.handle_arrange(session_id_, event.arrange());
		return;
	case v1::ClientEvent::kListDwellings:
		adapter_.handle_list_dwellings(session_id_);
		return;
	case v1::ClientEvent::kListSupplications:
		adapter_.handle_list_supplications(session_id_);
		return;
	case v1::ClientEvent::kRejectSupplication:
		adapter_.handle_reject_supplication(session_id_, event.reject_supplication());
		return;
	case v1::ClientEvent::kListRooms:
		adapter_.handle_list_rooms(session_id_, event.list_rooms());
		return;
	case v1::ClientEvent::kBear:
		adapter_.handle_bear(session_id_, event.bear());
		return;
	case v1::ClientEvent::kChooseFather:
		adapter_.handle_choose_father(session_id_, event.choose_father());
		return;
	case v1::ClientEvent::kListLineage:
		adapter_.handle_list_lineage(session_id_);
		return;
	case v1::ClientEvent::kTrain:
		adapter_.handle_train(session_id_, event.train());
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
	case v1::ClientEvent::kTurn:
	case v1::ClientEvent::kFulfil:
	case v1::ClientEvent::kAdmit:
	case v1::ClientEvent::kRegard:
	case v1::ClientEvent::kListDwellers:
	case v1::ClientEvent::kArrange:
	case v1::ClientEvent::kListDwellings:
	case v1::ClientEvent::kListSupplications:
	case v1::ClientEvent::kRejectSupplication:
	case v1::ClientEvent::kListRooms:
	case v1::ClientEvent::kBear:
	case v1::ClientEvent::kChooseFather:
	case v1::ClientEvent::kListLineage:
	case v1::ClientEvent::kTrain:
		adapter_.send_auth_required(session_id_);
		return;
	case v1::ClientEvent::EVENT_NOT_SET:
		break;
	}

	adapter_.close_with_protocol_error(session_id_, "Protocol error: unhandled client message type");
}


} // namespace will
