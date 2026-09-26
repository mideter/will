#include "protocoladapter.h"

#include "inbound_client_message_handler.h"

#include "beings/deed.h"
#include "beings/immanents/novice.h"
#include "beings/immanents/obedience.h"
#include "beings/immanents/shepherding.h"
#include "beings/immanents/testator.h"
#include "beings/immanents/tie.h"
#include "beings/immanents/witness.h"
#include "acts/tying.h"
#include "ports/temporality.h"
#include "values/device_token.h"

#include <exception>
#include <iostream>
#include <optional>
#include <string>
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
	if (const auto displaced = registry_.bind_vessel(session_id, man.Vessel::id()))
		close_session(*displaced);

	v1::ServerEvent event;
	event.mutable_auth_ok()->set_name(std::string{man.name().text()});
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
	const auto vessel_id = registry_.vessel_id(session_id);
	if (!vessel_id)
		return;

	const domain::Man& man = world_.man(world_.vessel(*vessel_id));
	const auto& witness = static_cast<const domain::Witness&>(man);
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
	for (const domain::Witness& observer : witness.abode().witnesses()) {
		if (observer.Vessel::id() == speaker_vessel)
			continue;
		send_to_vessel(observer.Vessel::id(), chat_event);
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
	const auto& witness = static_cast<const domain::Witness&>(man);

	std::vector<domain::Letter> letters;
	try {
		letters = witness.retell(request.limit());
	} catch (const std::exception&) {
		close_with_protocol_error(session_id, "Protocol error: invalid HistoryRequest");
		return;
	}

	const domain::id::Soul listener_soul = man.Soul::id();
	for (const domain::Letter& letter : letters) {
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

	try {
		testator.accept(testator.supplication(novice));
	} catch (const std::exception& e) {
		send_notice(session_id, e.what());
		return;
	}

	const auto tyings = self.temporality().tyings();
	domain::id::Tie tie_id{1};
	for (const domain::Tying& row : tyings) {
		if (row.testator() == self.Soul::id() && row.novice() == suppliant_man->Soul::id()) {
			tie_id = row.id();
			break;
		}
	}

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

	std::optional<domain::id::Tie> found;
	for (const domain::Tying& row : self.temporality().tyings()) {
		if (row.testator() == self.Soul::id() && row.novice() == novice_man->Soul::id()) {
			found = row.id();
			break;
		}
	}
	if (!found) {
		send_notice(session_id, "no obedience with that soul");
		return;
	}

	try {
		const domain::Deed deed = testator.will(testator.shepherding(*found), msg.body());

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
		const domain::Deed done =
			novice.execute(self.temporality().deed(domain::id::Deed{msg.deed_id()}));

		send_notice(session_id, "deed " + std::to_string(done.id().value()) + " done");

		v1::ServerEvent event;
		auto* row = event.mutable_deed_done();
		row->set_deed_id(done.id().value());
		row->set_novice_name(std::string{self.name().text()});
		send_to_vessel(done.tie().testator().Vessel::id(), event);
	} catch (const std::exception& e) {
		send_notice(session_id, e.what());
	}
}


} // namespace will
