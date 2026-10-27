/// The protocol adapter, its part of the abode: turning, what each place shows, the
/// rooms, the gates and the helm, the dwellers let in, regarded and told.
#include "protocoladapter.h"
#include "protocoladapter_wire.h"

#include "words/behest.h"
#include "words/deed.h"
#include "places/abode.h"
#include "words/training.h"
#include "men/man.h"
#include "men/novice.h"
#include "places/obedience.h"
#include "places/shepherding.h"
#include "men/testator.h"
#include "places/tie.h"
#include "places/room.h"
#include "places/gates.h"
#include "places/helm.h"
#include "places/part.h"
#include "relations/friend.h"
#include "relations/contemplation.h"
#include "relations/supplication.h"
#include "refusals/place.h"
#include "men/witness.h"
#include "words/letter.h"
#include "values/device_token.h"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <functional>
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
	const auto& witness = self;

	const domain::Man* host = &self;
	if (!msg.abode_of().empty()) {
		host = man_named(session_id, msg.abode_of());
		if (!host)
			return;
	}

	// Those in the gates he leaves, and in those he enters, see him go and come.
	const domain::Gates* left = gates_of_gaze(self);

	const domain::Abode& abode = host->abode();
	try {
		if (msg.has_edge()) {
			witness.contemplate(msg.edge() == v1::OUTER ? static_cast<const domain::Part&>(abode.outer())
														: abode.inner());
		} else if (!msg.room().empty()) {
			const domain::Room* room = abode.room(msg.room());
			if (!room) {
				refuse(session_id, v1::NOTICE_NO_SUCH_ROOM);
				return;
			}
			witness.contemplate(*room);
		} else {
			witness.contemplate(abode);
		}
	} catch (const domain::PassThroughGates& e) {
		refuse(session_id, e);
		return;
	} catch (const std::logic_error&) {
		refuse(session_id, msg.room().empty() && !msg.has_edge() ? v1::NOTICE_YOU_DO_NOT_DWELL
																  : v1::NOTICE_YOU_DO_NOT_ENTER);
		return;
	}

	v1::ServerEvent turned;
	// One's own abode is told unnamed, even when one names it.
	turned.mutable_turned()->set_abode_of(host == &self ? std::string{} : msg.abode_of());
	turned.mutable_turned()->set_writable(writes_here(self));
	if (const auto gaze = world_.contemplation(self))
		tell_standing(*turned.mutable_turned(), gaze->place());
	send_event(session_id, turned);

	if (const auto gaze = world_.contemplation(self))
		tell_view(session_id, *gaze, self, TurnWords);

	const domain::Gates* entered = gates_of_gaze(self);
	if (left && left != entered)
		retell_gates(*left, &self);
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

	send_notice(session_id, v1::NOTICE_ADMITTED, {std::string{man->name().text()}});
	tell_dwelling(self, *man);
	retell_gates(*gates);
	retell_helm(self.abode().inner().helm());
}


void ProtocolAdapter::handle_knock(const SessionId session_id)
{
	const domain::Man& self = session_man(session_id);
	const domain::Gates* gates = gates_of_gaze(self);
	try {
		const domain::Contemplation& gaze = gaze_of(self);
		gaze.gates().knock(gaze);
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	retell_gates(*gates);
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
		gaze.helm().regard(gaze, *man, kind);
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	send_notice(session_id, v1::NOTICE_REGARDED, {std::string{man->name().text()}});
	tell_dwelling(self, *man);
	retell_abode(self, *man);
	retell_helm(self.abode().inner().helm());
}


void ProtocolAdapter::retell_abode(const domain::Man& host, const domain::Man& dweller)
{
	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(dweller);
	if (!gaze)
		return;

	// He looks at the abode itself, or at one of its places.
	if (abode_of(gaze->place()) != &host.abode())
		return;

	const auto sid = registry_.session_id_for_vessel(dweller.Vessel::id());
	if (!sid)
		return;

	v1::ServerEvent turned;
	turned.mutable_turned()->set_abode_of(std::string{host.name().text()});
	turned.mutable_turned()->set_writable(writes_here(dweller));
	tell_standing(*turned.mutable_turned(), gaze->place());
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
	if (const auto* helm = dynamic_cast<const domain::Helm*>(&gaze.place())) {
		send_event(session_id, dwellers_event(helm->abode()));
		// The host sees here all the rooms of his abode, each in its part, and who asks him.
		send_event(session_id, arranged_rooms_event(helm->abode()));
		send_event(session_id, supplications_event(helm->abode().host()));
		v1::ServerEvent end_event;
		end_event.mutable_history_end();
		send_event(session_id, end_event);
		return;
	}

	const auto* abode = dynamic_cast<const domain::Abode*>(&gaze.place());
	if (!abode) {
		tell_words(session_id, gaze, listener, limit);
		// In a room of a tie, one is told the approaches underway in its trainings.
		for (const std::shared_ptr<const domain::Word>& word : gaze.room().words(gaze)) {
			const auto training = std::dynamic_pointer_cast<const domain::Training>(word);
			if (training && training->underway())
				send_event(session_id, underway_event(*training));
		}
		return;
	}

	tell_rooms(session_id, *abode, listener);

	if (&abode->host() == &listener)
		send_event(session_id, outstanding_event(*abode));

	v1::ServerEvent end_event;
	end_event.mutable_history_end();
	send_event(session_id, end_event);
}


void ProtocolAdapter::handle_arrange(const SessionId session_id, const v1::Arrange& msg)
{
	const domain::Man& self = session_man(session_id);
	const domain::Room* room = self.abode().room(msg.room());
	if (!room) {
		refuse(session_id, v1::NOTICE_NO_SUCH_ROOM);
		return;
	}

	const domain::Part& part = msg.part() == v1::OUTER ? static_cast<const domain::Part&>(self.abode().outer())
														: self.abode().inner();
	try {
		const domain::Contemplation& gaze = gaze_of(self);
		gaze.helm().arrange(gaze, *room, part);
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	send_notice(session_id,
				&part == &self.abode().outer() ? v1::NOTICE_ARRANGED_OUTER : v1::NOTICE_ARRANGED_INNER,
				{msg.room()});
	retell_helm(self.abode().inner().helm());  // the host there sees his rooms anew
	for (const std::shared_ptr<const domain::Acquaintance>& dweller : self.abode().dwellers())
		retell_abode(self, dweller->man());
	retell_gates(self.abode().outer().gates());  // set in another part, they are seen by other kinds
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
	for (const domain::Room& room : abode.rooms())
		tell_room(*rooms->add_rooms(), room);
	return event;
}


void ProtocolAdapter::tell_rooms(const SessionId session_id, const domain::Abode& abode,
								 const domain::Man& listener)
{
	v1::ServerEvent event;
	auto* rooms = event.mutable_rooms();
	for (const domain::Room& room : abode.rooms()) {
		if (room.dwells(listener))
			tell_room(*rooms->add_rooms(), room);
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

	const std::vector<std::reference_wrapper<const domain::Man>> knocking = gates.knocking();
	threshold->set_knocking(
		std::ranges::any_of(knocking, [&](const domain::Man& man) { return &man == &listener; }));

	// Only those who keep the gates see who waits to be let in.
	if (gates.keeps(listener)) {
		for (const domain::Man& man : knocking)
			threshold->add_waiting(std::string{man.name().text()});
	}
	return event;
}


v1::ServerEvent ProtocolAdapter::dwellers_event(const domain::Abode& abode) const
{
	const domain::Man& host = abode.host();

	// Whether the suppliant awaits the answer of the addressee.
	const auto asking = [](const domain::Man& suppliant, const domain::Man& addressee) {
		for (const std::shared_ptr<const domain::Supplication>& pending :
			 addressee.supplications()) {
			if (pending->suppliant().Soul::id() == suppliant.Soul::id())
				return true;
		}
		return false;
	};
	// Whether the trainer shepherds the novice in a tie.
	const auto shepherds = [](const domain::Man& trainer, const domain::Man& novice) {
		for (const domain::Obedience& tie : novice.obediences()) {
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
	for (const domain::Man& man : world_.contemplating(gates)) {
		if (&man == except)
			continue;
		send_to_vessel(man.Vessel::id(), threshold_event(gates, man));
		// One who now keeps them sees the unborn as well.
		if (gates.keeps(man))
			send_to_vessel(man.Vessel::id(), unborn_event());
	}
	// The host looking at his abode sees who waits at its gates there too; one who moves
	// himself is told his view anew as he turns.
	if (&gates.abode().host() != except)
		tell_overview(gates.abode().host());
}


void ProtocolAdapter::retell_helm(const domain::Helm& helm)
{
	const v1::ServerEvent event = dwellers_event(helm.abode());
	for (const domain::Man& man : world_.contemplating(helm)) {
		send_to_vessel(man.Vessel::id(), event);
		// A tie bound or let go changes the rooms too.
		send_to_vessel(man.Vessel::id(), arranged_rooms_event(helm.abode()));
		send_to_vessel(man.Vessel::id(), supplications_event(helm.abode().host()));
	}
}

v1::ServerEvent ProtocolAdapter::outstanding_event(const domain::Abode& abode) const
{
	const domain::Man& host = abode.host();
	v1::ServerEvent event;
	auto* outstanding = event.mutable_outstanding();
	const auto tell = [&](const std::vector<std::shared_ptr<const domain::Behest>>& behests, auto&& add) {
		for (const std::shared_ptr<const domain::Behest>& behest : behests) {
			v1::OutstandingBehest* told = add();
			if (const domain::Room* room = abode.room(behest->tie()))
				told->set_room(room->name());
			told->set_counterpart(std::string{behest->tie().counterpart(host).name().text()});
			*told->mutable_behest() = word_event(*behest, host.Soul::id()).word();
			const auto training = std::dynamic_pointer_cast<const domain::Training>(behest);
			if (training && training->underway())
				*told->mutable_underway() = underway_event(*training).underway();
		}
	};
	tell(abode.owed(), [&] { return outstanding->add_owed(); });
	tell(abode.awaited(), [&] { return outstanding->add_awaited(); });
	tell(abode.unreviewed(), [&] { return outstanding->add_unreviewed(); });
	outstanding->set_waiting(static_cast<std::uint32_t>(abode.outer().gates().knocking().size()));
	return event;
}


void ProtocolAdapter::tell_overview(const domain::Tie& tie)
{
	// A side looking at his own abode sees there what is outstanding: as the tie changes, anew.
	for (const domain::Man* side : {&tie.testator(), &tie.novice()})
		tell_overview(*side);
}


void ProtocolAdapter::tell_overview(const domain::Man& host)
{
	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(host);
	if (gaze && &gaze->place() == static_cast<const domain::Place*>(&host.abode()))
		send_to_vessel(host.Vessel::id(), outstanding_event(host.abode()));
}


} // namespace will
