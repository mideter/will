#include "protocoladapter.h"

#include "inbound_client_message_handler.h"

#include "words/behest.h"
#include "words/deed.h"
#include "places/abode.h"
#include "men/novice.h"
#include "places/obedience.h"
#include "places/shepherding.h"
#include "men/testator.h"
#include "places/tie.h"
#include "places/room.h"
#include "relations/friend.h"
#include "relations/contemplation.h"
#include "men/witness.h"
#include "words/letter.h"
#include "values/device_token.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>


namespace will {
namespace {


/// The soul whose word this is.
const domain::Soul& author_of(const domain::Word& word)
{
	if (const auto* letter = dynamic_cast<const domain::Letter*>(&word))
		return letter->author();
	if (const auto* behest = dynamic_cast<const domain::Behest*>(&word))
		return behest->tie().testator();
	return dynamic_cast<const domain::Deed&>(word).tie().novice();
}


/// A word as the wire tells it to one who listens.
v1::ServerEvent word_event(const domain::Word& word, const domain::id::Soul listener)
{
	const domain::Soul& author = author_of(word);

	v1::ServerEvent event;
	auto* told = event.mutable_word();
	told->set_id(word.id().value());
	told->set_is_mine(author.id() == listener);
	told->set_name(std::string{author.name().text()});
	told->set_body(word.saying().body());

	if (dynamic_cast<const domain::Behest*>(&word)) {
		told->set_kind(v1::Word::BEHEST);
	} else if (const auto* deed = dynamic_cast<const domain::Deed*>(&word)) {
		told->set_kind(v1::Word::DEED);
		told->set_behest_id(deed->behest().value());
	} else {
		told->set_kind(v1::Word::LETTER);
	}

	return event;
}


/// The other side of a tie, seen from one of its sides.
const domain::Man& counterpart_in(const domain::Tie& tie, const domain::Man& side)
{
	if (tie.testator().Soul::id() == side.Soul::id())
		return tie.novice();
	return tie.testator();
}


} // namespace


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


void ProtocolAdapter::send_notice(const SessionId session_id, const std::string_view message)
{
	v1::ServerEvent event;
	event.mutable_protocol_notice()->set_message(std::string{message});
	send_event(session_id, event);
}


void ProtocolAdapter::send_to_vessel(const domain::id::Vessel vessel_id, const v1::ServerEvent& event)
{
	if (const auto sid = registry_.session_id_for_vessel(vessel_id))
		send_event(*sid, event);
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


const domain::Man& ProtocolAdapter::session_man(const SessionId session_id)
{
	const auto vessel_id = registry_.vessel_id(session_id);
	if (!vessel_id)
		throw std::logic_error("session has no vessel");

	return world_.man(world_.vessel(*vessel_id));
}


const domain::Man* ProtocolAdapter::man_named(const SessionId session_id, const std::string_view name_text)
{
	const auto name = domain::SoulName::parse(name_text);
	if (!name) {
		send_notice(session_id, "invalid soul name");
		return nullptr;
	}

	try {
		return &world_.man(*name);
	} catch (const std::invalid_argument&) {
		send_notice(session_id, "unknown soul name");
		return nullptr;
	}
}


void ProtocolAdapter::handle_bind_token(const SessionId session_id, const v1::BindToken& token)
{
	const auto device_token = domain::DeviceToken::parse(token.token());
	if (!device_token) {
		send_auth_required(session_id);
		return;
	}

	const domain::Man& man = world_.welcome(*device_token);

	std::optional<SessionId> displaced;
	{
		std::lock_guard lock(presence_mutex_);
		displaced = registry_.bind_vessel(session_id, man.Vessel::id());
		static_cast<const domain::Witness&>(man).wake();
		woken_by_session_.insert_or_assign(session_id.value, man.Vessel::id());
	}

	if (displaced)
		close_session(*displaced);

	v1::ServerEvent event;
	event.mutable_auth_ok()->set_name(std::string{man.name().text()});
	send_event(session_id, event);
}


void ProtocolAdapter::on_session_ended(const SessionId session_id)
{
	std::lock_guard lock(presence_mutex_);

	const auto it = woken_by_session_.find(session_id.value);
	if (it == woken_by_session_.end())
		return;

	const domain::id::Vessel vessel_id = it->second;
	woken_by_session_.erase(it);

	// A newer session of the same body keeps the man awake.
	const std::optional<SessionId> current = registry_.session_id_for_vessel(vessel_id);
	if (current && *current != session_id)
		return;

	static_cast<const domain::Witness&>(world_.man(world_.vessel(vessel_id))).sleep();
}


void ProtocolAdapter::send_auth_required(const SessionId session_id)
{
	v1::ServerEvent event;
	event.mutable_auth_required();
	send_event(session_id, event);
}


void ProtocolAdapter::handle_user_chat(const SessionId session_id, const v1::ChatMessage& chat)
{
	const auto vessel_id = registry_.vessel_id(session_id);
	if (!vessel_id)
		return;

	const domain::Man& man = world_.man(world_.vessel(*vessel_id));

	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(man.Soul::id());
	if (!gaze) {
		close_with_protocol_error(session_id, "Protocol error: invalid ChatMessage");
		return;
	}

	std::shared_ptr<const domain::Word> placed;
	if (dynamic_cast<const domain::Abode*>(&gaze->place())) {
		send_notice(session_id, "one writes in a room: /room Келья");
		return;
	}
	if (const auto* abode = dynamic_cast<const domain::Abode*>(&gaze->place().source())) {
		if (&abode->host() != &man) {
			send_notice(session_id, "only the host writes in his abode");
			return;
		}

		try {
			man.say(std::string{chat.body()});
		} catch (const std::exception&) {
			close_with_protocol_error(session_id, "Protocol error: invalid ChatMessage");
			return;
		}
		placed = gaze->words().back();
	} else {
		// In a tie the testator wills; the novice fulfils with /done.
		const auto& tie = dynamic_cast<const domain::Tie&>(gaze->place().source());
		if (tie.testator().Soul::id() != man.Soul::id()) {
			send_notice(session_id, "in a tie the novice fulfils behests: /done <number> [report]");
			return;
		}

		try {
			placed = static_cast<const domain::Testator&>(man).will(tie, std::string{chat.body()});
		} catch (const std::exception& e) {
			send_notice(session_id, e.what());
			return;
		}
	}

	v1::ServerEvent ack;
	ack.mutable_receipt_ack();
	send_event(session_id, ack);

	tell_placed(gaze->place().source(), *placed, man);
}


void ProtocolAdapter::tell_placed(const domain::Place& place, const domain::Word& word, const domain::Man& author)
{
	for (const domain::Soul& beholder : world_.contemplating(place)) {
		const auto& man = static_cast<const domain::Man&>(beholder);
		if (man.Soul::id() == author.Soul::id())
			continue;

		// Each sees it as the place he looks at shows it: the place, or a room reflecting it.
		const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(man.Soul::id());
		if (gaze && gaze->place().shows(man, word))
			send_to_vessel(man.Vessel::id(), word_event(word, man.Soul::id()));
	}

	// The other side of a tie, looking elsewhere, hears a short word of it.
	const auto* tie = dynamic_cast<const domain::Tie*>(&place);
	if (!tie)
		return;

	const domain::Man& other = counterpart_in(*tie, author);
	for (const domain::Soul& beholder : world_.contemplating(place)) {
		if (beholder.id() == other.Soul::id())
			return;
	}

	v1::ServerEvent event;
	auto* stirred = event.mutable_stirred();
	stirred->set_tie_with(std::string{author.name().text()});
	stirred->set_author_name(std::string{author.name().text()});
	send_to_vessel(other.Vessel::id(), event);
}


void ProtocolAdapter::tell_words(const SessionId session_id, const domain::Contemplation& gaze,
								 const domain::Man& listener, const std::uint32_t limit)
{
	const std::vector<std::shared_ptr<const domain::Word>> words = gaze.words();

	// How many words to send is the client's wish, not the place's concern.
	const std::size_t first = words.size() > limit ? words.size() - limit : 0;
	for (std::size_t i = first; i < words.size(); ++i)
		send_event(session_id, word_event(*words[i], listener.Soul::id()));

	v1::ServerEvent end_event;
	end_event.mutable_history_end();
	send_event(session_id, end_event);
}


void ProtocolAdapter::handle_history_request(const SessionId session_id, const v1::HistoryRequest& request)
{
	const auto vessel_id = registry_.vessel_id(session_id);
	if (!vessel_id)
		return;

	const domain::Man& man = world_.man(world_.vessel(*vessel_id));

	if (request.limit() == 0) {
		close_with_protocol_error(session_id, "Protocol error: invalid HistoryRequest");
		return;
	}

	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(man.Soul::id());
	if (!gaze) {
		close_with_protocol_error(session_id, "Protocol error: invalid HistoryRequest");
		return;
	}

	tell_view(session_id, *gaze, man, request.limit());
}


void ProtocolAdapter::handle_supplicate(const SessionId session_id, const v1::Supplicate& msg)
{
	const domain::Man* addressee_man = man_named(session_id, msg.addressee_name());
	if (!addressee_man)
		return;

	const domain::Man& self = session_man(session_id);
	const auto& novice = static_cast<const domain::Novice&>(self);
	const auto& testator = static_cast<const domain::Testator&>(*addressee_man);

	try {
		novice.supplicate(testator);
	} catch (const std::exception& e) {
		send_notice(session_id, e.what());
		return;
	}

	send_notice(session_id, "supplication sent");

	v1::ServerEvent offer;
	offer.mutable_supplication_offer()->set_suppliant_name(std::string{self.name().text()});
	send_to_vessel(addressee_man->Vessel::id(), offer);
}


void ProtocolAdapter::handle_accept_supplication(const SessionId session_id,
												 const v1::AcceptSupplication& msg)
{
	const domain::Man* suppliant_man = man_named(session_id, msg.suppliant_name());
	if (!suppliant_man)
		return;

	const domain::Man& self = session_man(session_id);
	const auto& testator = static_cast<const domain::Testator&>(self);
	const auto& novice = static_cast<const domain::Novice&>(*suppliant_man);

	std::optional<domain::id::Place> formed;
	try {
		formed = testator.accept(*testator.supplication(novice)).id();
	} catch (const std::exception& e) {
		send_notice(session_id, e.what());
		return;
	}

	const domain::id::Place tie_id = *formed;

	v1::ServerEvent to_testator;
	auto* formed_t = to_testator.mutable_tie_formed();
	formed_t->set_tie_id(tie_id.value());
	formed_t->set_counterpart_name(std::string{suppliant_man->name().text()});
	formed_t->set_as_novice(false);
	send_event(session_id, to_testator);

	v1::ServerEvent to_novice;
	auto* formed_n = to_novice.mutable_tie_formed();
	formed_n->set_tie_id(tie_id.value());
	formed_n->set_counterpart_name(std::string{self.name().text()});
	formed_n->set_as_novice(true);
	send_to_vessel(suppliant_man->Vessel::id(), to_novice);
}


void ProtocolAdapter::handle_turn(const SessionId session_id, const v1::Turn& msg)
{
	const domain::Man& self = session_man(session_id);
	const auto& witness = static_cast<const domain::Witness&>(self);

	if (msg.tie_with().empty()) {
		const domain::Man* host = &self;
		if (!msg.abode_of().empty()) {
			host = man_named(session_id, msg.abode_of());
			if (!host)
				return;
		}

		const domain::Place* place = &host->abode();
		if (!msg.room().empty()) {
			place = host->abode().room(msg.room());
			if (!place) {
				send_notice(session_id, "no such room");
				return;
			}
		}

		try {
			witness.contemplate(*place);
		} catch (const std::logic_error&) {
			send_notice(session_id, msg.room().empty() ? "you do not dwell in that abode"
													   : "you do not enter that room");
			return;
		}
	} else {
		const domain::Man* counterpart = man_named(session_id, msg.tie_with());
		if (!counterpart)
			return;

		const domain::Place* tie = nullptr;
		try {
			tie = &static_cast<const domain::Testator&>(self).shepherding(
				static_cast<const domain::Novice&>(*counterpart));
		} catch (const std::invalid_argument&) {
			try {
				tie = &static_cast<const domain::Novice&>(self).obedience(
					static_cast<const domain::Testator&>(*counterpart));
			} catch (const std::invalid_argument&) {
				send_notice(session_id, "no tie with that soul");
				return;
			}
		}

		witness.contemplate(*tie);
	}

	v1::ServerEvent turned;
	turned.mutable_turned()->set_tie_with(msg.tie_with());
	turned.mutable_turned()->set_abode_of(msg.abode_of());
	turned.mutable_turned()->set_room(msg.room());
	send_event(session_id, turned);

	if (const auto gaze = world_.contemplation(self.Soul::id()))
		tell_view(session_id, *gaze, self, TurnWords);
}


void ProtocolAdapter::handle_fulfil(const SessionId session_id, const v1::Fulfil& msg)
{
	const domain::Man& self = session_man(session_id);
	const auto& novice = static_cast<const domain::Novice&>(self);

	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(self.Soul::id());
	if (!gaze || !dynamic_cast<const domain::Tie*>(&gaze->place().source())) {
		send_notice(session_id, "turn to the tie first: /tie <name>");
		return;
	}

	// The number a client names is matched here, at the border, among the words he beholds.
	std::shared_ptr<const domain::Behest> behest;
	for (const std::shared_ptr<const domain::Word>& word : gaze->words()) {
		if (word->id().value() == msg.behest_id())
			behest = std::dynamic_pointer_cast<const domain::Behest>(word);
	}
	if (!behest) {
		send_notice(session_id, "unknown behest");
		return;
	}

	try {
		std::optional<domain::Saying> report;
		if (!msg.report().empty())
			report.emplace(msg.report());

		const std::shared_ptr<const domain::Deed> deed = novice.execute(*behest, std::move(report));
		send_notice(session_id, "behest " + std::to_string(behest->id().value()) + " fulfilled");
		tell_placed(gaze->place().source(), *deed, self);
	} catch (const std::exception& e) {
		send_notice(session_id, e.what());
	}
}


namespace {


v1::DwellerKind kind_of(const domain::Acquaintance& dweller)
{
	if (dynamic_cast<const domain::Friend*>(&dweller))
		return v1::FRIEND;
	if (dynamic_cast<const domain::Neighbour*>(&dweller))
		return v1::NEIGHBOUR;
	return v1::ACQUAINTANCE;
}


} // namespace


void ProtocolAdapter::tell_dwelling(const domain::Man& host, const domain::Man& dweller)
{
	const std::shared_ptr<const domain::Acquaintance> regarded = host.abode().dweller(dweller);
	if (!regarded)
		return;

	v1::ServerEvent event;
	auto* dwelling = event.mutable_dwelling();
	dwelling->set_host_name(std::string{host.name().text()});
	dwelling->set_kind(kind_of(*regarded));
	send_to_vessel(dweller.Vessel::id(), event);
}


void ProtocolAdapter::handle_admit(const SessionId session_id, const v1::Admit& msg)
{
	const domain::Man* man = man_named(session_id, msg.name());
	if (!man)
		return;

	const domain::Man& self = session_man(session_id);
	try {
		self.admit(*man);
	} catch (const std::exception& e) {
		send_notice(session_id, e.what());
		return;
	}

	send_notice(session_id, std::string{man->name().text()} + " admitted as an acquaintance");
	tell_dwelling(self, *man);
}


void ProtocolAdapter::handle_regard(const SessionId session_id, const v1::Regard& msg)
{
	const domain::Man* man = man_named(session_id, msg.name());
	if (!man)
		return;

	domain::matter::Dweller::Kind kind = domain::matter::Dweller::Kind::Acquaintance;
	switch (msg.kind()) {
	case v1::ACQUAINTANCE:
		break;
	case v1::NEIGHBOUR:
		kind = domain::matter::Dweller::Kind::Neighbour;
		break;
	case v1::FRIEND:
		kind = domain::matter::Dweller::Kind::Friend;
		break;
	default:
		send_notice(session_id, "unknown dweller kind");
		return;
	}

	const domain::Man& self = session_man(session_id);
	try {
		self.regard(*man, kind);
	} catch (const std::exception& e) {
		send_notice(session_id, e.what());
		return;
	}

	send_notice(session_id, std::string{man->name().text()} + " regarded anew");
	tell_dwelling(self, *man);
	retell_abode(self, *man);
}


void ProtocolAdapter::retell_abode(const domain::Man& host, const domain::Man& dweller)
{
	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(dweller.Soul::id());
	if (!gaze)
		return;

	// He looks at the abode itself, or at one of its rooms.
	const auto* room = dynamic_cast<const domain::Room*>(&gaze->place());
	const bool here = &gaze->place() == static_cast<const domain::Place*>(&host.abode())
					  || (room && &room->abode() == &host.abode());
	if (!here)
		return;

	const auto sid = registry_.session_id_for_vessel(dweller.Vessel::id());
	if (!sid)
		return;

	v1::ServerEvent turned;
	turned.mutable_turned()->set_abode_of(std::string{host.name().text()});
	if (room)
		turned.mutable_turned()->set_room(room->name());
	send_event(*sid, turned);
	tell_view(*sid, *gaze, dweller, TurnWords);
}


void ProtocolAdapter::handle_list_dwellers(const SessionId session_id)
{
	const domain::Man& self = session_man(session_id);

	v1::ServerEvent event;
	auto* list = event.mutable_dwellers();
	for (const std::shared_ptr<const domain::Acquaintance>& dweller : self.abode().dwellers()) {
		auto* told = list->add_dwellers();
		told->set_name(std::string{dweller->man().name().text()});
		told->set_kind(kind_of(*dweller));
	}
	send_event(session_id, event);
}


void ProtocolAdapter::tell_view(const SessionId session_id, const domain::Contemplation& gaze,
								const domain::Man& listener, const std::uint32_t limit)
{
	const auto* abode = dynamic_cast<const domain::Abode*>(&gaze.place());
	if (!abode) {
		tell_words(session_id, gaze, listener, limit);
		return;
	}

	v1::ServerEvent rooms_event;
	auto* rooms = rooms_event.mutable_rooms();
	for (const domain::Room& room : abode->rooms()) {
		if (!room.dwells(listener))
			continue;
		auto* told = rooms->add_rooms();
		told->set_name(room.name());
		told->set_part(room.part() == domain::matter::Room::Part::Outer ? v1::OUTER : v1::INNER);
	}
	send_event(session_id, rooms_event);

	if (&abode->host() == &listener) {
		v1::ServerEvent outstanding_event;
		auto* outstanding = outstanding_event.mutable_outstanding();
		for (const std::shared_ptr<const domain::Behest>& behest : abode->outstanding()) {
			auto* told = outstanding->add_behests();
			if (const domain::Room* room = abode->room(behest->tie()))
				told->set_room(room->name());
			*told->mutable_behest() = word_event(*behest, listener.Soul::id()).word();
		}
		send_event(session_id, outstanding_event);
	}

	v1::ServerEvent end_event;
	end_event.mutable_history_end();
	send_event(session_id, end_event);
}


void ProtocolAdapter::handle_arrange(const SessionId session_id, const v1::Arrange& msg)
{
	const domain::Man& self = session_man(session_id);
	const domain::Room* room = self.abode().room(msg.room());
	if (!room) {
		send_notice(session_id, "no such room");
		return;
	}

	const auto part = msg.part() == v1::OUTER ? domain::matter::Room::Part::Outer : domain::matter::Room::Part::Inner;
	try {
		self.arrange(*room, part);
	} catch (const std::exception& e) {
		send_notice(session_id, e.what());
		return;
	}

	send_notice(session_id, msg.room() + (part == domain::matter::Room::Part::Outer ? " is now in the outer part"
																				  : " is now in the inner part"));
	for (const std::shared_ptr<const domain::Acquaintance>& dweller : self.abode().dwellers())
		retell_abode(self, dweller->man());
}


} // namespace will
