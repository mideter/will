#include "protocoladapter.h"
#include "protocoladapter_wire.h"

#include "inbound_client_message_handler.h"

#include "places/abode.h"
#include "men/man.h"
#include "places/room.h"
#include "places/gates.h"
#include "places/helm.h"
#include "places/part.h"
#include "relations/contemplation.h"
#include "words/letter.h"
#include "values/device_token.h"
#include "refusals/refusal.h"

#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>


namespace will {


using namespace wire;


ProtocolAdapter::ProtocolAdapter(domain::World& world, SessionRegistry& registry)
	: world_(world)
	, registry_(registry)
{}


void ProtocolAdapter::on_client_event(const SessionId session_id, const v1::ClientEvent& event)
{
	InboundClientMessageHandler handler{*this, session_id};
	if (event.request_id() == 0) {
		handler.on(event);
		return;
	}

	// A numbered request is answered once, by its number.
	{
		std::lock_guard lock(answering_mutex_);
		answering_.insert_or_assign(session_id.value, Answer{event.request_id()});
	}
	try {
		handler.on(event);
	} catch (...) {
		std::lock_guard lock(answering_mutex_);
		answering_.erase(session_id.value);
		throw;
	}

	Answer answer;
	{
		std::lock_guard lock(answering_mutex_);
		if (auto node = answering_.extract(session_id.value))
			answer = std::move(node.mapped());
	}
	v1::ServerEvent reply;
	reply.mutable_reply()->set_request_id(answer.request_id);
	reply.mutable_reply()->set_granted(!answer.refused);
	reply.mutable_reply()->set_notice(answer.notice);
	reply.mutable_reply()->set_code(answer.code);
	for (std::string& arg : answer.args)
		reply.mutable_reply()->add_args(std::move(arg));
	send_event(session_id, reply);
}


bool ProtocolAdapter::is_authenticated(const SessionId session_id) const
{
	return registry_.is_authenticated(session_id);
}


void ProtocolAdapter::send_event(const SessionId session_id, const v1::ServerEvent& event)
{
	registry_.enqueue_event(session_id, event);
}


void ProtocolAdapter::send_notice(const SessionId session_id, const v1::NoticeCode code,
								  std::vector<std::string> args)
{
	std::string message = words_of(code, args);
	tell_asker(session_id, code, std::move(args), std::move(message), false);
}


void ProtocolAdapter::refuse(const SessionId session_id, const v1::NoticeCode code, std::vector<std::string> args)
{
	std::string message = words_of(code, args);
	tell_asker(session_id, code, std::move(args), std::move(message), true);
}


void ProtocolAdapter::refuse(const SessionId session_id, const std::exception& thrown)
{
	const auto* refusal = dynamic_cast<const domain::Refusal*>(&thrown);
	tell_asker(session_id, refusal ? code_of(*refusal) : v1::NOTICE_UNSPECIFIED, {}, thrown.what(), true);
}


void ProtocolAdapter::tell_asker(const SessionId session_id, const v1::NoticeCode code, std::vector<std::string> args,
								 std::string message, const bool refusal)
{
	{
		std::lock_guard lock(answering_mutex_);
		if (const auto it = answering_.find(session_id.value); it != answering_.end()) {
			Answer& answer = it->second;
			answer.refused = answer.refused || refusal;
			if (!answer.told && (code != v1::NOTICE_UNSPECIFIED || !message.empty())) {
				answer.told = true;
				answer.code = code;
				answer.args = std::move(args);
				answer.notice = std::move(message);
			}
			return;
		}
	}
	if (message.empty())
		return;
	v1::ServerEvent event;
	auto* notice = event.mutable_protocol_notice();
	notice->set_message(message);
	notice->set_code(code);
	for (std::string& arg : args)
		notice->add_args(std::move(arg));
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
		refuse(session_id, v1::NOTICE_INVALID_NAME);
		return nullptr;
	}

	try {
		return &world_.man(*name);
	} catch (const std::invalid_argument&) {
		refuse(session_id, v1::NOTICE_UNKNOWN_NAME);
		return nullptr;
	}
}


void ProtocolAdapter::send_auth_required(const SessionId session_id)
{
	tell_asker(session_id, v1::NOTICE_UNSPECIFIED, {}, {}, true);  // a numbered request before the token is not granted
	v1::ServerEvent event;
	event.mutable_auth_required();
	send_event(session_id, event);
}


bool ProtocolAdapter::writes_here(const domain::Man& man) const
{
	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(man);
	return gaze && gaze->writable();
}


const domain::Contemplation& ProtocolAdapter::gaze_of(const domain::Man& man) const
{
	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(man);
	if (!gaze)
		throw std::logic_error("he contemplates nothing");
	return *gaze;
}


const domain::Gates* ProtocolAdapter::gates_of_gaze(const domain::Man& man) const
{
	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(man);
	return gaze ? dynamic_cast<const domain::Gates*>(&gaze->place()) : nullptr;
}


} // namespace will
