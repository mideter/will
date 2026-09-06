#pragma once

#include "serverconfig.h"
#include "sessionparticipantnotifierimpl.h"
#include "sessionregistry.h"

#include "ports/messenger_persistence.h"

#include "infra/transport/messenger.pb.h"

#include <string_view>


namespace will {


class InboundClientMessageHandler;


/** Maps client protobuf events to domain communion and enqueues outbound events. */
class ProtocolAdapter {
	friend class InboundClientMessageHandler;

public:
	ProtocolAdapter(domain::MessengerPersistence persistence, SessionRegistry& registry);

	void on_client_event(SessionId session_id, const v1::ClientEvent& event);

	bool is_authenticated(SessionId session_id) const;

private:
	void handle_bind_token(SessionId session_id, const v1::BindToken& token);
	void handle_user_chat(SessionId session_id, const v1::ChatMessage& chat);
	void handle_history_request(SessionId session_id, const v1::HistoryRequest& request);
	void send_auth_required(SessionId session_id);
	void send_event(SessionId session_id, const v1::ServerEvent& event);
	void close_with_protocol_error(SessionId session_id, std::string_view message);
	void close_session(SessionId session_id);

	domain::MessengerPersistence persistence_;
	SessionRegistry& registry_;
	SessionParticipantNotifierImpl participant_notifier_;
};


} // namespace will
