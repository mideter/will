/// The protocol adapter, its part of the abode: turning, what each place shows, the
/// rooms, the gates and the upper room, the dwellers let in, regarded and told.
#include "protocoladapter.h"
#include "protocoladapter_wire.h"

#include "inbound_client_message_handler.h"

#include "words/behest.h"
#include "words/deed.h"
#include "words/training.h"
#include "places/abode.h"
#include "men/novice.h"
#include "places/obedience.h"
#include "places/shepherding.h"
#include "men/testator.h"
#include "places/tie.h"
#include "places/room.h"
#include "relations/friend.h"
#include "relations/contemplation.h"
#include "relations/supplication.h"
#include "men/witness.h"
#include "words/letter.h"
#include "values/device_token.h"
#include "refusals/refusal.h"

#include <algorithm>
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


using namespace wire;


void ProtocolAdapter::handle_turn(const SessionId session_id, const v1::Turn& msg)
{
	const domain::Man& self = session_man(session_id);
	const auto& witness = static_cast<const domain::Witness&>(self);

	const domain::Man* host = &self;
	if (!msg.abode_of().empty()) {
		host = man_named(session_id, msg.abode_of());
		if (!host)
			return;
	}

	// Those in the gates he leaves, and in those he enters, see him go and come.
	const domain::Gates* left = gates_of_gaze(self);

	try {
		if (msg.room().empty()) {
			witness.contemplate(host->abode());
		} else {
			const domain::Room* room = host->abode().room(msg.room());
			if (!room) {
				refuse(session_id, v1::NOTICE_NO_SUCH_ROOM);
				return;
			}
			witness.contemplate(*room);
		}
	} catch (const std::logic_error&) {
		refuse(session_id, msg.room().empty() ? v1::NOTICE_YOU_DO_NOT_DWELL : v1::NOTICE_YOU_DO_NOT_ENTER);
		return;
	}

	v1::ServerEvent turned;
	turned.mutable_turned()->set_abode_of(msg.abode_of());
	turned.mutable_turned()->set_room(msg.room());
	turned.mutable_turned()->set_writable(writes_here(self));
	if (const auto gaze = world_.contemplation(self.Soul::id()))
		turned.mutable_turned()->set_aspect(aspect_of(gaze->place()));
	send_event(session_id, turned);

	if (const auto gaze = world_.contemplation(self.Soul::id()))
		tell_view(session_id, *gaze, self, TurnWords);

	const domain::Gates* entered = gates_of_gaze(self);
	if (left && left != entered)
		retell_gates(*left);
	if (entered)
		retell_gates(*entered, &self);
}


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
	const domain::Gates* gates = gates_of_gaze(self);
	try {
		const domain::Contemplation& gaze = gaze_of(self);
		gaze.gates().admit(gaze, *man);
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	// He becomes an acquaintance of the host of the gates, whoever let him in.
	const domain::Man& host = gates->abode().host();
	send_notice(session_id, v1::NOTICE_ADMITTED, {std::string{man->name().text()}});
	if (&host != &self) {
		const std::vector<std::string> names{std::string{self.name().text()}, std::string{man->name().text()}};
		v1::ServerEvent told;
		auto* notice = told.mutable_protocol_notice();
		notice->set_message(words_of(v1::NOTICE_LET_IN, names));
		notice->set_code(v1::NOTICE_LET_IN);
		for (const std::string& name : names)
			notice->add_args(name);
		send_to_vessel(host.Vessel::id(), told);
	}
	tell_dwelling(host, *man);
	retell_gates(*gates);
	retell_upper_room(host.abode().upper_room());
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
		refuse(session_id, v1::NOTICE_UNKNOWN_KIND);
		return;
	}

	const domain::Man& self = session_man(session_id);
	try {
		const domain::Contemplation& gaze = gaze_of(self);
		gaze.upper_room().regard(gaze, *man, kind);
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	// He regards in the upper room he stands in, as its host.
	const domain::Man& host = host_of_upper_room(self);
	send_notice(session_id, v1::NOTICE_REGARDED, {std::string{man->name().text()}});
	tell_dwelling(host, *man);
	retell_abode(host, *man);
	retell_upper_room(host.abode().upper_room());
	retell_gates(host.abode().gates());  // his kind may let him keep them now, or no longer
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
	turned.mutable_turned()->set_writable(writes_here(dweller));
	turned.mutable_turned()->set_aspect(aspect_of(gaze->place()));
	send_event(*sid, turned);
	tell_view(*sid, *gaze, dweller, TurnWords);
}


void ProtocolAdapter::handle_list_dwellers(const SessionId session_id)
{
	const domain::Man& self = session_man(session_id);

	send_event(session_id, dwellers_event(self.abode()));
}


void ProtocolAdapter::tell_view(const SessionId session_id, const domain::Contemplation& gaze,
								const domain::Man& listener, const std::uint32_t limit)
{
	if (const auto* gates = dynamic_cast<const domain::Gates*>(&gaze.place())) {
		send_event(session_id, threshold_event(*gates, listener));
		// Who keeps the gates sees the unborn too: he may bear one through them.
		if (gates->keeps(listener))
			send_event(session_id, unborn_event());
		v1::ServerEvent end_event;
		end_event.mutable_history_end();
		send_event(session_id, end_event);
		return;
	}
	if (const auto* upper_room = dynamic_cast<const domain::UpperRoom*>(&gaze.place())) {
		send_event(session_id, dwellers_event(upper_room->abode()));
		// Whoever stands here arranges the abode as its host: he is shown all its rooms, each in its part.
		send_event(session_id, arranged_rooms_event(upper_room->abode()));
		v1::ServerEvent end_event;
		end_event.mutable_history_end();
		send_event(session_id, end_event);
		return;
	}

	const auto* abode = dynamic_cast<const domain::Abode*>(&gaze.place());
	if (!abode) {
		tell_words(session_id, gaze, listener, limit);
		// In a room of a tie, one is told what its novice is doing now.
		if (const auto* tie = dynamic_cast<const domain::Tie*>(&gaze.place().source())) {
			if (static_cast<const domain::Novice&>(tie->novice()).underway())
				send_event(session_id, underway_event(*tie));
		}
		return;
	}

	tell_rooms(session_id, *abode, listener);

	if (&abode->host() == &listener) {
		v1::ServerEvent outstanding_event;
		auto* outstanding = outstanding_event.mutable_outstanding();
		const auto tell = [&](const std::vector<std::shared_ptr<const domain::Behest>>& behests, auto&& add) {
			for (const std::shared_ptr<const domain::Behest>& behest : behests) {
				v1::OutstandingBehest* told = add();
				if (const domain::Cell* cell = abode->cell(behest->tie()))
					told->set_room(cell->name());
				*told->mutable_behest() = word_event(*behest, listener.Soul::id()).word();
			}
		};
		tell(abode->owed(), [&] { return outstanding->add_owed(); });
		tell(abode->awaited(), [&] { return outstanding->add_awaited(); });
		send_event(session_id, outstanding_event);
	}

	v1::ServerEvent end_event;
	end_event.mutable_history_end();
	send_event(session_id, end_event);
}


void ProtocolAdapter::handle_arrange(const SessionId session_id, const v1::Arrange& msg)
{
	const domain::Man& self = session_man(session_id);
	const domain::Man& host = host_of_upper_room(self);
	const domain::Room* room = host.abode().room(msg.room());
	if (!room) {
		refuse(session_id, v1::NOTICE_NO_SUCH_ROOM);
		return;
	}

	const auto part = msg.part() == v1::OUTER ? domain::matter::Room::Part::Outer : domain::matter::Room::Part::Inner;
	try {
		const domain::Contemplation& gaze = gaze_of(self);
		gaze.upper_room().arrange(gaze, *room, part);
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	send_notice(session_id,
				part == domain::matter::Room::Part::Outer ? v1::NOTICE_ARRANGED_OUTER : v1::NOTICE_ARRANGED_INNER,
				{msg.room()});
	send_event(session_id, arranged_rooms_event(host.abode()));
	for (const std::shared_ptr<const domain::Acquaintance>& dweller : host.abode().dwellers())
		retell_abode(host, dweller->man());
	retell_gates(host.abode().gates());  // set in another part, they are kept by other kinds
}


void ProtocolAdapter::handle_list_dwellings(const SessionId session_id)
{
	const domain::Man& self = session_man(session_id);

	v1::ServerEvent event;
	auto* list = event.mutable_dwellings();
	for (const domain::Man& host : world_.hosts_of(self)) {
		const std::shared_ptr<const domain::Acquaintance> regarded = host.abode().dweller(self);
		if (!regarded)
			continue;
		auto* told = list->add_dwellings();
		told->set_host_name(std::string{host.name().text()});
		told->set_kind(kind_of(*regarded));
	}
	send_event(session_id, event);
}


v1::ServerEvent ProtocolAdapter::arranged_rooms_event(const domain::Abode& abode) const
{
	v1::ServerEvent event;
	auto* rooms = event.mutable_rooms();
	for (const domain::Room& room : abode.rooms()) {
		auto* told = rooms->add_rooms();
		told->set_name(room.name());
		told->set_part(room.part() == domain::matter::Room::Part::Outer ? v1::OUTER : v1::INNER);
	}
	return event;
}


void ProtocolAdapter::tell_rooms(const SessionId session_id, const domain::Abode& abode,
								 const domain::Man& listener)
{
	v1::ServerEvent event;
	auto* rooms = event.mutable_rooms();
	for (const domain::Room& room : abode.rooms()) {
		if (!room.dwells(listener))
			continue;
		auto* told = rooms->add_rooms();
		told->set_name(room.name());
		told->set_part(room.part() == domain::matter::Room::Part::Outer ? v1::OUTER : v1::INNER);
	}
	send_event(session_id, event);
}


void ProtocolAdapter::handle_list_rooms(const SessionId session_id, const v1::ListRooms& msg)
{
	const domain::Man& self = session_man(session_id);
	const domain::Man* host = &self;
	if (!msg.abode_of().empty()) {
		host = man_named(session_id, msg.abode_of());
		if (!host)
			return;
	}
	if (!host->abode().dwells(self)) {
		refuse(session_id, v1::NOTICE_YOU_DO_NOT_DWELL);
		return;
	}

	tell_rooms(session_id, host->abode(), self);
}


v1::ServerEvent ProtocolAdapter::threshold_event(const domain::Gates& gates, const domain::Man& listener) const
{
	v1::ServerEvent event;
	auto* threshold = event.mutable_threshold();
	threshold->set_open(gates.open());
	threshold->set_keeping(gates.keeps(listener));

	// Only those who keep the gates see who waits to be let in: those at them who do not dwell here.
	if (gates.keeps(listener)) {
		for (const domain::Soul& soul : world_.contemplating(gates)) {
			const auto& man = static_cast<const domain::Man&>(soul);
			if (&man != &listener && !gates.abode().dweller(man))
				threshold->add_waiting(std::string{man.name().text()});
		}
	}
	return event;
}


v1::ServerEvent ProtocolAdapter::dwellers_event(const domain::Abode& abode) const
{
	const domain::Man& host = abode.host();

	// Whether the suppliant awaits the answer of the addressee.
	const auto asking = [](const domain::Man& suppliant, const domain::Man& addressee) {
		for (const std::shared_ptr<const domain::Supplication>& pending :
			 static_cast<const domain::Testator&>(addressee).supplications()) {
			if (pending->suppliant().Soul::id() == suppliant.Soul::id())
				return true;
		}
		return false;
	};
	// Whether the trainer shepherds the novice in a tie.
	const auto shepherds = [](const domain::Man& trainer, const domain::Man& novice) {
		for (const domain::Obedience& tie : static_cast<const domain::Novice&>(novice).obediences()) {
			if (tie.testator().Soul::id() == trainer.Soul::id())
				return true;
		}
		return false;
	};

	v1::ServerEvent event;
	auto* list = event.mutable_dwellers();
	for (const std::shared_ptr<const domain::Acquaintance>& dweller : abode.dwellers()) {
		const domain::Man& man = dweller->man();
		auto* told = list->add_dwellers();
		told->set_name(std::string{man.name().text()});
		told->set_kind(kind_of(*dweller));
		told->set_asked(asking(host, man));
		told->set_asks(asking(man, host));
		told->set_trainer(shepherds(man, host));
		told->set_novice(shepherds(host, man));
	}
	return event;
}


void ProtocolAdapter::retell_gates(const domain::Gates& gates, const domain::Soul* except)
{
	for (const domain::Soul& soul : world_.contemplating(gates)) {
		if (&soul == except)
			continue;
		const auto& man = static_cast<const domain::Man&>(soul);
		send_to_vessel(man.Vessel::id(), threshold_event(gates, man));
		// One who now keeps them sees the unborn as well.
		if (gates.keeps(man))
			send_to_vessel(man.Vessel::id(), unborn_event());
	}
}


void ProtocolAdapter::retell_upper_room(const domain::UpperRoom& upper_room)
{
	const v1::ServerEvent event = dwellers_event(upper_room.abode());
	for (const domain::Soul& soul : world_.contemplating(upper_room)) {
		const auto& man = static_cast<const domain::Man&>(soul);
		send_to_vessel(man.Vessel::id(), event);
		// Whoever stands here arranges: a tie bound or let go changes the rooms too.
		send_to_vessel(man.Vessel::id(), arranged_rooms_event(upper_room.abode()));
	}
}


} // namespace will
