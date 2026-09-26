#pragma once

#include "serverconfig.h"
#include "sessionregistry.h"

#include "beings/world.h"

#include "infra/transport/messenger.pb.h"

#include <string_view>


namespace will {


class InboundClientMessageHandler;


/** Maps client protobuf events to domain communion and enqueues outbound events. */
class ProtocolAdapter {
	friend class InboundClientMessageHandler;

public:
	ProtocolAdapter(domain::World& world, SessionRegistry& registry);

	void on_client_event(SessionId session_id, const v1::ClientEvent& event);

	[[nodiscard]] bool is_authenticated(SessionId session_id) const;

private:
	void handle_bind_token(SessionId session_id, const v1::BindToken& token);
	void handle_user_chat(SessionId session_id, const v1::ChatMessage& chat);
	void handle_history_request(SessionId session_id, const v1::HistoryRequest& request);
	void handle_supplicate(SessionId session_id, const v1::Supplicate& msg);
	void handle_accept_supplication(SessionId session_id, const v1::AcceptSupplication& msg);
	void handle_will_deed(SessionId session_id, const v1::WillDeed& msg);
	void handle_execute_deed(SessionId session_id, const v1::ExecuteDeed& msg);

	void send_auth_required(SessionId session_id);
	void send_event(SessionId session_id, const v1::ServerEvent& event);
	void send_notice(SessionId session_id, std::string_view message);
	void send_to_vessel(domain::id::Vessel vessel_id, const v1::ServerEvent& event);
	void close_with_protocol_error(SessionId session_id, std::string_view message);
	void close_session(SessionId session_id);

	const domain::Man* man_named(SessionId session_id, std::string_view name_text);
	const domain::Man& session_man(SessionId session_id);

	domain::World& world_;
	SessionRegistry& registry_;
};


} // namespace will
