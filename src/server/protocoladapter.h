#pragma once

#include "serverconfig.h"
#include "sessionregistry.h"

#include "horizons/world.h"

#include "infra/transport/messenger.pb.h"

#include <cstdint>
#include <exception>
#include <mutex>
#include <vector>
#include <string_view>
#include <unordered_map>


namespace will {


namespace domain {
class Tie;
class Training;
class Gates;
class Helm;
} // namespace domain


class InboundClientMessageHandler;


/// Maps client protobuf events to domain communion and enqueues outbound events. A session
/// ended puts its man asleep, unless a newer session of his body took over; waking and
/// falling asleep are guarded against the two meeting. What came of a request goes into its
/// Reply if the asker numbered it, else into a notice — by its code, the names apart, and in
/// the server's own words; the world's Refusal is told by its reason. A word newly placed is
/// told to those who look at the place, and a short word of it to the other side of a tie
/// who looks elsewhere. Turning to a place, one is told what one sees there: at an abode
/// itself, the rooms one may enter and, to its host, what awaits him; elsewhere, the last
/// TurnWords words. One writes in one's own cell, or in a tie room as its testator.
class ProtocolAdapter {
	friend class InboundClientMessageHandler;

public:
	ProtocolAdapter(domain::World& world, SessionRegistry& registry);

	void on_client_event(SessionId session_id, const v1::ClientEvent& event);

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
	void handle_bear(SessionId session_id, const v1::Bear& msg);
	void handle_choose_father(SessionId session_id, const v1::ChooseFather& msg);
	void handle_list_lineage(SessionId session_id);
	void handle_train(SessionId session_id, const v1::Train& msg);
	void handle_begin_approach(SessionId session_id, const v1::BeginApproach& msg);
	void handle_finish_approach(SessionId session_id, const v1::FinishApproach& msg);

	std::shared_ptr<const domain::Training> training_in_gaze(SessionId session_id, const domain::Man& man,
															 std::uint64_t behest_id);

	void tell_tie(const domain::Tie& tie, const v1::ServerEvent& event);

	v1::ServerEvent underway_event(const domain::Training& training) const;

	bool awaits_birth(SessionId session_id) const;

	void tell_placed(const domain::Place& place, const domain::Word& word, const domain::Man& author);

	void tell_words(SessionId session_id, const domain::Contemplation& gaze, const domain::Man& listener,
					std::uint32_t limit);

	static constexpr std::uint32_t TurnWords = 50;
	static constexpr std::uint32_t MaxHistoryWords = 500;

	void send_auth_required(SessionId session_id);
	void send_event(SessionId session_id, const v1::ServerEvent& event);
	void send_notice(SessionId session_id, v1::NoticeCode code, std::vector<std::string> args = {});
	void refuse(SessionId session_id, v1::NoticeCode code, std::vector<std::string> args = {});
	void refuse(SessionId session_id, const std::exception& thrown);
	void tell_asker(SessionId session_id, v1::NoticeCode code, std::vector<std::string> args, std::string message,
					bool refusal);
	void send_to_vessel(domain::id::Vessel vessel_id, const v1::ServerEvent& event);
	void close_with_protocol_error(SessionId session_id, std::string_view message);
	void close_session(SessionId session_id);

	const domain::Man* man_named(SessionId session_id, std::string_view name_text);

	void tell_dwelling(const domain::Man& host, const domain::Man& dweller);

	bool writes_here(const domain::Man& man) const;

	const domain::Contemplation& gaze_of(const domain::Man& man) const;


	void retell_abode(const domain::Man& host, const domain::Man& dweller);

	void retell_gates(const domain::Gates& gates, const domain::Soul* except = nullptr);

	void retell_helm(const domain::Helm& helm);

	void retell_unborn();

	v1::ServerEvent unborn_event() const;

	v1::ServerEvent threshold_event(const domain::Gates& gates, const domain::Man& listener) const;

	v1::ServerEvent dwellers_event(const domain::Abode& abode) const;

	const domain::Gates* gates_of_gaze(const domain::Man& man) const;

	v1::ServerEvent arranged_rooms_event(const domain::Abode& abode) const;

	v1::ServerEvent supplications_event(const domain::Man& addressee) const;

	void tell_rooms(SessionId session_id, const domain::Abode& abode, const domain::Man& listener);

	void tell_view(SessionId session_id, const domain::Contemplation& gaze, const domain::Man& listener,
				   std::uint32_t limit);
	const domain::Man& session_man(SessionId session_id);

	domain::World& world_;
	SessionRegistry& registry_;

	std::mutex presence_mutex_;
	std::unordered_map<std::uint64_t, domain::id::Vessel> woken_by_session_;

	/// A numbered request being answered: what is said to the asker meanwhile goes into
	/// its one Reply, sent when the request is handled.
	struct Answer {
		std::uint64_t request_id = 0;
		bool refused = false;
		bool told = false;
		v1::NoticeCode code = v1::NOTICE_UNSPECIFIED;
		std::vector<std::string> args;
		std::string notice;
	};
	std::mutex answering_mutex_;
	std::unordered_map<std::uint64_t, Answer> answering_;
};


} // namespace will
