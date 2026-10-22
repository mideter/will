/// The protocol adapter, its part of coming in and going away: the token bound, the
/// session ended, the unborn told and borne, the father by spirit.
#include "protocoladapter.h"
#include "protocoladapter_wire.h"

#include "places/abode.h"
#include "men/man.h"
#include "places/room.h"
#include "relations/contemplation.h"
#include "men/witness.h"
#include "words/letter.h"
#include "values/device_token.h"

#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>


namespace will {


using namespace wire;


void ProtocolAdapter::handle_bind_token(const SessionId session_id, const v1::BindToken& token)
{
	const auto device_token = domain::DeviceToken::parse(token.token());
	if (!device_token) {
		send_auth_required(session_id);
		return;
	}

	const domain::Vessel& body = world_.welcome(*device_token);
	const auto* man = dynamic_cast<const domain::Man*>(&body);

	std::optional<SessionId> displaced;
	{
		std::lock_guard lock(presence_mutex_);
		displaced = registry_.bind_vessel(session_id, body.id());
		if (man) {
			man->wake();
			woken_by_session_.insert_or_assign(session_id.value, body.id());
		}
	}

	if (displaced)
		close_session(*displaced);

	v1::ServerEvent event;
	if (man) {
		event.mutable_auth_ok()->set_name(std::string{man->name().text()});
	} else {
		event.mutable_auth_ok()->set_unborn(true);
		event.mutable_auth_ok()->set_mark(body.id().value());
	}
	event.mutable_auth_ok()->set_server_time_ns(world_.now().value());
	send_event(session_id, event);

	// The keepers standing in gates see the new body awaiting.
	if (!man)
		retell_unborn();
}


bool ProtocolAdapter::awaits_birth(const SessionId session_id) const
{
	const auto vessel_id = registry_.vessel_id(session_id);
	return vessel_id && dynamic_cast<const domain::Unborn*>(&world_.vessel(*vessel_id));
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

	const domain::Man& man = world_.man(world_.vessel(vessel_id));
	const domain::Gates* left = gates_of_gaze(man);
	man.sleep();
	if (left)
		retell_gates(*left);
}


void ProtocolAdapter::handle_bear(const SessionId session_id, const v1::Bear& msg)
{
	const domain::Man& midwife = session_man(session_id);

	const domain::Unborn* unborn = nullptr;
	try {
		unborn = &world_.unborn(domain::id::Vessel{msg.mark()});
	} catch (const std::exception&) {
		refuse(session_id, v1::NOTICE_NO_SUCH_UNBORN);
		return;
	}

	const domain::Man* born = nullptr;
	try {
		const domain::Contemplation& gaze = gaze_of(midwife);
		born = &gaze.gates().bear(gaze, *unborn);
	} catch (const std::logic_error& e) {
		refuse(session_id, e);
		return;
	}
	const domain::Man& child = *born;
	const domain::Man& father = *child.father(domain::matter::Fatherhood::Line::Flesh);

	// The body that waited comes into the world: it wakes in its own abode.
	if (const auto child_session = registry_.session_id_for_vessel(child.Vessel::id())) {
		{
			std::lock_guard lock(presence_mutex_);
			child.wake();
			woken_by_session_.insert_or_assign(child_session->value, child.Vessel::id());
		}
		v1::ServerEvent event;
		event.mutable_auth_ok()->set_name(std::string{child.name().text()});
		event.mutable_auth_ok()->set_server_time_ns(world_.now().value());
		send_event(*child_session, event);
	}

	send_notice(session_id, v1::NOTICE_BORN, {std::string{child.name().text()}});
	tell_dwelling(father, child);
	tell_dwelling(child, father);
	retell_upper_room(father.abode().upper_room());
	retell_unborn();
}


void ProtocolAdapter::handle_choose_father(const SessionId session_id, const v1::ChooseFather&)
{
	// The father by spirit is not yet open: the domain keeps him, the wire does not yet lead to him.
	refuse(session_id, v1::NOTICE_FATHER_BY_SPIRIT_CLOSED);
}


void ProtocolAdapter::handle_list_lineage(const SessionId session_id)
{
	refuse(session_id, v1::NOTICE_FATHER_BY_SPIRIT_CLOSED);
}


v1::ServerEvent ProtocolAdapter::unborn_event() const
{
	v1::ServerEvent event;
	auto* unborn = event.mutable_unborn();
	for (const domain::Unborn& body : world_.unborn())
		unborn->add_marks(body.id().value());
	return event;
}


void ProtocolAdapter::retell_unborn()
{
	std::vector<domain::id::Vessel> woken;
	{
		std::lock_guard lock(presence_mutex_);
		for (const auto& [session, vessel] : woken_by_session_)
			woken.push_back(vessel);
	}

	const v1::ServerEvent event = unborn_event();
	for (const domain::id::Vessel vessel : woken) {
		const auto* man = dynamic_cast<const domain::Man*>(&world_.vessel(vessel));
		if (!man)
			continue;
		const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(*man);
		const auto* gates = gaze ? dynamic_cast<const domain::Gates*>(&gaze->place()) : nullptr;
		if (gates && gates->keeps(*man))
			send_to_vessel(vessel, event);
	}
}


} // namespace will
