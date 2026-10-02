#include "protocoladapter.h"

#include "inbound_client_message_handler.h"

#include "words/deed.h"
#include "immanents/novice.h"
#include "immanents/obedience.h"
#include "immanents/shepherding.h"
#include "immanents/testator.h"
#include "immanents/tie.h"
#include "immanents/contemplation.h"
#include "immanents/witness.h"
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


/// The number a client names in /done is matched here, at the border: the
/// domain shows the deeds of a place, it does not look words up by id.
std::optional<domain::Deed> deed_numbered(const domain::Novice& novice, const std::uint64_t number)
{
	for (const domain::Obedience& obedience : novice.obediences()) {
		for (domain::Deed& deed : obedience.deeds(novice)) {
			if (deed.id().value() == number)
				return std::move(deed);
		}
	}

	return std::nullopt;
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

	try {
		man.say(std::string{chat.body()});
	} catch (const std::exception&) {
		close_with_protocol_error(session_id, "Protocol error: invalid ChatMessage");
		return;
	}

	std::string author_name{man.name().text()};
	v1::ServerEvent chat_event;
	auto* chat_message = chat_event.mutable_chat();
	chat_message->set_name(author_name);
	chat_message->set_body(chat.body());

	const domain::id::Vessel speaker_vessel = man.Vessel::id();
	for (const domain::Soul& observer : world_.contemplating(gaze->abode())) {
		const auto& observer_man = static_cast<const domain::Man&>(observer);
		if (observer_man.Vessel::id() == speaker_vessel)
			continue;
		send_to_vessel(observer_man.Vessel::id(), chat_event);
	}

	v1::ServerEvent ack;
	ack.mutable_receipt_ack();
	send_event(session_id, ack);
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

	std::vector<domain::Letter> letters;
	try {
		letters = gaze->letters();
	} catch (const std::exception&) {
		close_with_protocol_error(session_id, "Protocol error: invalid HistoryRequest");
		return;
	}

	// How many letters to send is the client's wish, not the abode's concern.
	const std::size_t first = letters.size() > request.limit() ? letters.size() - request.limit() : 0;

	const domain::id::Soul listener_soul = man.Soul::id();
	for (std::size_t i = first; i < letters.size(); ++i) {
		const domain::Letter& letter = letters[i];

		v1::ServerEvent event;
		auto* history_item = event.mutable_history_item();
		history_item->set_message_id(letter.id().value());
		history_item->set_is_mine(letter.author().id() == listener_soul);
		history_item->set_name(std::string{letter.author().name().text()});
		history_item->set_body(letter.saying().body());
		send_event(session_id, event);
	}

	v1::ServerEvent end_event;
	end_event.mutable_history_end();
	send_event(session_id, end_event);
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

	std::optional<domain::id::Tie> formed;
	try {
		formed = domain::id::Tie{testator.accept(testator.supplication(novice)).id()};
	} catch (const std::exception& e) {
		send_notice(session_id, e.what());
		return;
	}

	const domain::id::Tie tie_id = *formed;

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


void ProtocolAdapter::handle_will_deed(const SessionId session_id, const v1::WillDeed& msg)
{
	const domain::Man* novice_man = man_named(session_id, msg.novice_name());
	if (!novice_man)
		return;

	const domain::Man& self = session_man(session_id);
	const auto& testator = static_cast<const domain::Testator&>(self);

	const auto& novice = static_cast<const domain::Novice&>(*novice_man);

	const domain::Shepherding* shepherding = nullptr;
	try {
		shepherding = &testator.shepherding(novice);
	} catch (const std::invalid_argument&) {
		send_notice(session_id, "no obedience with that soul");
		return;
	}

	try {
		const domain::Deed deed = testator.will(*shepherding, msg.body());

		send_notice(session_id, "deed " + std::to_string(deed.id().value()) + " willed");

		v1::ServerEvent offered;
		auto* row = offered.mutable_deed_offered();
		row->set_deed_id(deed.id().value());
		row->set_testator_name(std::string{self.name().text()});
		row->set_body(deed.saying().body());
		send_to_vessel(novice_man->Vessel::id(), offered);
	} catch (const std::exception& e) {
		send_notice(session_id, e.what());
	}
}


void ProtocolAdapter::handle_execute_deed(const SessionId session_id, const v1::ExecuteDeed& msg)
{
	const domain::Man& self = session_man(session_id);
	const auto& novice = static_cast<const domain::Novice&>(self);

	try {
		const std::optional<domain::Deed> deed = deed_numbered(novice, msg.deed_id());
		if (!deed)
			throw std::invalid_argument("unknown deed");

		novice.execute(*deed);

		send_notice(session_id, "deed " + std::to_string(deed->id().value()) + " done");

		v1::ServerEvent event;
		auto* row = event.mutable_deed_done();
		row->set_deed_id(deed->id().value());
		row->set_novice_name(std::string{self.name().text()});
		send_to_vessel(deed->tie().testator().Vessel::id(), event);
	} catch (const std::exception& e) {
		send_notice(session_id, e.what());
	}
}


} // namespace will
