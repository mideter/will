#pragma once

#include "serverconfig.h"
#include "sessionregistry.h"

#include "horizons/world.h"

#include "infra/transport/messenger.pb.h"

#include <cstdint>
#include <mutex>
#include <string_view>
#include <unordered_map>


namespace will {


class InboundClientMessageHandler;


/** Maps client protobuf events to domain communion and enqueues outbound events. */
class ProtocolAdapter {
	friend class InboundClientMessageHandler;

public:
	ProtocolAdapter(domain::World& world, SessionRegistry& registry);

	void on_client_event(SessionId session_id, const v1::ClientEvent& event);

	/// The stream of this session is over, for whatever reason. The man whose
	/// body it served falls asleep, unless a newer session of that body took over.
	void on_session_ended(SessionId session_id);

	[[nodiscard]] bool is_authenticated(SessionId session_id) const;

private:
	void handle_bind_token(SessionId session_id, const v1::BindToken& token);
	void handle_user_chat(SessionId session_id, const v1::ChatMessage& chat);
	void handle_history_request(SessionId session_id, const v1::HistoryRequest& request);
	void handle_supplicate(SessionId session_id, const v1::Supplicate& msg);
	void handle_accept_supplication(SessionId session_id, const v1::AcceptSupplication& msg);
	void handle_turn(SessionId session_id, const v1::Turn& msg);
	void handle_fulfil(SessionId session_id, const v1::Fulfil& msg);
	void handle_admit(SessionId session_id, const v1::Admit& msg);
	void handle_regard(SessionId session_id, const v1::Regard& msg);
	void handle_list_dwellers(SessionId session_id);
	void handle_arrange(SessionId session_id, const v1::Arrange& msg);
	void handle_list_dwellings(SessionId session_id);
	void handle_list_rooms(SessionId session_id, const v1::ListRooms& msg);
	void handle_list_supplications(SessionId session_id);
	void handle_reject_supplication(SessionId session_id, const v1::RejectSupplication& msg);

	/// Tell a word newly placed to those who contemplate the place, and a short
	/// word of it to the other side of a tie who looks elsewhere.
	void tell_placed(const domain::Place& place, const domain::Word& word, const domain::Man& author);

	/// Tell the last words of what one contemplates, then the end of them.
	void tell_words(SessionId session_id, const domain::Contemplation& gaze, const domain::Man& listener,
					std::uint32_t limit);

	/// How many words are told on turning to a place.
	static constexpr std::uint32_t TurnWords = 50;

	void send_auth_required(SessionId session_id);
	void send_event(SessionId session_id, const v1::ServerEvent& event);
	void send_notice(SessionId session_id, std::string_view message);
	void send_to_vessel(domain::id::Vessel vessel_id, const v1::ServerEvent& event);
	void close_with_protocol_error(SessionId session_id, std::string_view message);
	void close_session(SessionId session_id);

	const domain::Man* man_named(SessionId session_id, std::string_view name_text);

	/// Tell a dweller how the host regards him now.
	void tell_dwelling(const domain::Man& host, const domain::Man& dweller);

	/// Whether this man may write where he looks: his own cell, or a tie room of
	/// which he is the testator.
	bool writes_here(const domain::Man& man) const;

	/// A dweller looking at the host's abode sees it anew, as his new kind does.
	void retell_abode(const domain::Man& host, const domain::Man& dweller);

	/// Tell the rooms of this abode that this man may enter.
	void tell_rooms(SessionId session_id, const domain::Abode& abode, const domain::Man& listener);

	/// Tell what the gaze sees: at an abode itself, the rooms one may enter and,
	/// to its host, what awaits him; elsewhere, the words.
	void tell_view(SessionId session_id, const domain::Contemplation& gaze, const domain::Man& listener,
				   std::uint32_t limit);
	const domain::Man& session_man(SessionId session_id);

	domain::World& world_;
	SessionRegistry& registry_;

	/// Guards waking and falling asleep against a session ending while another
	/// of the same body begins.
	std::mutex presence_mutex_;
	std::unordered_map<std::uint64_t, domain::id::Vessel> woken_by_session_;
};


} // namespace will
