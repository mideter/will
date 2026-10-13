#include "protocoladapter.h"

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
#include "values/refusal.h"

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
namespace {


v1::RoomAspect aspect_of(const domain::Place& place);


/// The soul whose word this is.
const domain::Soul& author_of(const domain::Word& word)
{
	if (const auto* letter = dynamic_cast<const domain::Letter*>(&word))
		return letter->author();
	if (const auto* behest = dynamic_cast<const domain::Behest*>(&word))
		return behest->tie().testator();
	return dynamic_cast<const domain::Deed&>(word).tie().novice();
}


/// Exercises as the wire tells them.
void tell_exercises(const std::vector<domain::Exercise>& exercises,
					google::protobuf::RepeatedPtrField<v1::Exercise>& told)
{
	for (const domain::Exercise& exercise : exercises) {
		v1::Exercise* wire = told.Add();
		wire->set_name(exercise.name());
		for (const domain::Approach& approach : exercise.approaches()) {
			v1::Approach* go = wire->add_approaches();
			go->set_weight_grams(approach.weight().grams());
			go->set_repetitions(approach.repetitions());
			go->set_rest_seconds(approach.rest_seconds());
		}
	}
}


/// Exercises as the wire brings them. Throws std::invalid_argument if any is not one.
std::vector<domain::Exercise> exercises_of(const google::protobuf::RepeatedPtrField<v1::Exercise>& wire)
{
	std::vector<domain::Exercise> exercises;
	exercises.reserve(static_cast<std::size_t>(wire.size()));
	for (const v1::Exercise& exercise : wire) {
		std::vector<domain::Approach> approaches;
		for (const v1::Approach& go : exercise.approaches())
			approaches.emplace_back(domain::Weight{go.weight_grams()}, go.repetitions(), go.rest_seconds());
		exercises.emplace_back(exercise.name(), std::move(approaches));
	}
	return exercises;
}


/// An effort as the wire tells it.
void tell_effort(const domain::matter::Effort& effort, v1::Effort& told)
{
	told.set_exercise(effort.exercise());
	told.set_approach(effort.approach());
	told.set_weight_grams(effort.weight().grams());
	told.set_repetitions(effort.repetitions());
	told.set_begun_at_ns(effort.begun().value());
	told.set_finished_at_ns(effort.finished().value());
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
		if (const auto* training = dynamic_cast<const domain::Training*>(&word)) {
			tell_exercises(training->exercises(), *told->mutable_exercises());
			for (const domain::matter::Effort& effort : training->efforts())
				tell_effort(effort, *told->add_efforts());
		}
	} else if (const auto* deed = dynamic_cast<const domain::Deed*>(&word)) {
		told->set_kind(v1::Word::DEED);
		told->set_behest_id(deed->behest().value());
		tell_exercises(deed->performed(), *told->mutable_exercises());
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


/// The server's own words for what it tells by a code, the names in it given apart.
std::string words_of(const v1::NoticeCode code, const std::vector<std::string>& args)
{
	const auto arg = [&](const std::size_t i) { return i < args.size() ? args[i] : std::string{}; };
	switch (code) {
	case v1::NOTICE_NO_SUCH_UNBORN:
		return "no such unborn";
	case v1::NOTICE_ONLY_HOST_WRITES:
		return "only the host writes in his abode";
	case v1::NOTICE_INVALID_NAME:
		return "invalid soul name";
	case v1::NOTICE_UNKNOWN_NAME:
		return "unknown soul name";
	case v1::NOTICE_WRITE_IN_A_ROOM:
		return "one writes in a room: /room Слово";
	case v1::NOTICE_NO_WRITING_HERE:
		return "one does not write here";
	case v1::NOTICE_NOVICE_FULFILS:
		return "in a tie the novice fulfils behests: /done <number> [report]";
	case v1::NOTICE_NO_SUCH_ROOM:
		return "no such room";
	case v1::NOTICE_YOU_DO_NOT_DWELL:
		return "you do not dwell in that abode";
	case v1::NOTICE_YOU_DO_NOT_ENTER:
		return "you do not enter that room";
	case v1::NOTICE_ENTER_TIE_ROOM:
		return "enter the room of the tie first: /room <name>";
	case v1::NOTICE_UNKNOWN_BEHEST:
		return "unknown behest";
	case v1::NOTICE_UNKNOWN_KIND:
		return "unknown dweller kind";
	case v1::NOTICE_ONLY_TESTATOR_TRAINS:
		return "only the testator wills a training";
	case v1::NOTICE_INVALID_TRAINING:
		return "invalid training";
	case v1::NOTICE_UNKNOWN_TRAINING:
		return "unknown training";
	case v1::NOTICE_FATHER_BY_SPIRIT_CLOSED:
		return "the father by spirit is not yet open";
	case v1::NOTICE_AWAITS_BIRTH:
		return "one awaits one's birth";
	case v1::NOTICE_SUPPLICATION_SENT:
		return "supplication sent";
	case v1::NOTICE_BEHEST_FULFILLED:
		return "behest " + arg(0) + " fulfilled";
	case v1::NOTICE_ADMITTED:
		return arg(0) + " admitted as an acquaintance";
	case v1::NOTICE_LET_IN:
		return arg(0) + " let " + arg(1) + " in as an acquaintance";
	case v1::NOTICE_REGARDED:
		return arg(0) + " regarded anew";
	case v1::NOTICE_ARRANGED_OUTER:
		return arg(0) + " is now in the outer part";
	case v1::NOTICE_ARRANGED_INNER:
		return arg(0) + " is now in the inner part";
	case v1::NOTICE_TRAINING_WILLED:
		return "training " + arg(0) + " willed";
	case v1::NOTICE_BORN:
		return arg(0) + " born";
	case v1::NOTICE_SUPPLICATION_REJECTED:
		return "supplication of " + arg(0) + " rejected";
	default:
		return "refused";
	}
}


/// The code by which the world's reason for refusing is told.
v1::NoticeCode code_of(const domain::Refusal::Reason reason)
{
	using Reason = domain::Refusal::Reason;
	switch (reason) {
	case Reason::AdmitOnlyAtGates: return v1::NOTICE_ADMIT_ONLY_AT_GATES;
	case Reason::KeepGatesByKind: return v1::NOTICE_KEEP_GATES_BY_KIND;
	case Reason::AlreadyDwells: return v1::NOTICE_ALREADY_DWELLS;
	case Reason::NotAtGates: return v1::NOTICE_NOT_AT_GATES;
	case Reason::BearOnlyAtGates: return v1::NOTICE_BEAR_ONLY_AT_GATES;
	case Reason::BearOnlyAtKeptGates: return v1::NOTICE_BEAR_ONLY_AT_KEPT_GATES;
	case Reason::NoSuchUnborn: return v1::NOTICE_NO_SUCH_UNBORN;
	case Reason::DoesNotDwell: return v1::NOTICE_DOES_NOT_DWELL;
	case Reason::DoesNotEnterRoom: return v1::NOTICE_DOES_NOT_ENTER_ROOM;
	case Reason::RegardOnlyInUpperRoom: return v1::NOTICE_REGARD_ONLY_IN_UPPER_ROOM;
	case Reason::ArrangeOnlyInUpperRoom: return v1::NOTICE_ARRANGE_ONLY_IN_UPPER_ROOM;
	case Reason::FatherByFleshNeighbour: return v1::NOTICE_FATHER_BY_FLESH_NEIGHBOUR;
	case Reason::FatherBySpiritFriend: return v1::NOTICE_FATHER_BY_SPIRIT_FRIEND;
	case Reason::OnlyHostWrites: return v1::NOTICE_ONLY_HOST_WRITES;
	case Reason::SayOnlyInWord: return v1::NOTICE_SAY_ONLY_IN_WORD;
	case Reason::CannotSupplicateOneself: return v1::NOTICE_CANNOT_SUPPLICATE_ONESELF;
	case Reason::SupplicateOnlyDweller: return v1::NOTICE_SUPPLICATE_ONLY_DWELLER;
	case Reason::HearOnlyDweller: return v1::NOTICE_HEAR_ONLY_DWELLER;
	case Reason::AlreadyTied: return v1::NOTICE_ALREADY_TIED;
	case Reason::TieOnlyMutualDwellers: return v1::NOTICE_TIE_ONLY_MUTUAL_DWELLERS;
	case Reason::UnknownSupplication: return v1::NOTICE_UNKNOWN_SUPPLICATION;
	case Reason::WillOnlyInContemplatedTie: return v1::NOTICE_WILL_ONLY_IN_CONTEMPLATED_TIE;
	case Reason::OnlyTestatorWills: return v1::NOTICE_ONLY_TESTATOR_WILLS;
	case Reason::FulfilOnlyInContemplatedTie: return v1::NOTICE_FULFIL_ONLY_IN_CONTEMPLATED_TIE;
	case Reason::OnlyNoviceFulfils: return v1::NOTICE_ONLY_NOVICE_FULFILS;
	case Reason::AlreadyFulfilled: return v1::NOTICE_ALREADY_FULFILLED;
	case Reason::PlainBehestWithoutExercises: return v1::NOTICE_PLAIN_BEHEST_WITHOUT_EXERCISES;
	case Reason::TrainOnlyInContemplatedTie: return v1::NOTICE_TRAIN_ONLY_IN_CONTEMPLATED_TIE;
	case Reason::OnlyNoviceTrains: return v1::NOTICE_ONLY_NOVICE_TRAINS;
	case Reason::NoSuchExercise: return v1::NOTICE_NO_SUCH_EXERCISE;
	case Reason::NoSuchApproach: return v1::NOTICE_NO_SUCH_APPROACH;
	case Reason::ApproachAlreadyDone: return v1::NOTICE_APPROACH_ALREADY_DONE;
	case Reason::ApproachesInOrder: return v1::NOTICE_APPROACHES_IN_ORDER;
	case Reason::AnotherApproachUnderway: return v1::NOTICE_ANOTHER_APPROACH_UNDERWAY;
	case Reason::NoApproachUnderway: return v1::NOTICE_NO_APPROACH_UNDERWAY;
	case Reason::FinishApproachFirst: return v1::NOTICE_FINISH_APPROACH_FIRST;
	case Reason::NotOwnFather: return v1::NOTICE_NOT_OWN_FATHER;
	case Reason::NotDescendantAsFather: return v1::NOTICE_NOT_DESCENDANT_AS_FATHER;
	}
	return v1::NOTICE_UNSPECIFIED;
}


} // namespace


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
	tell_asker(session_id, refusal ? code_of(refusal->reason()) : v1::NOTICE_UNSPECIFIED, {}, thrown.what(), true);
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
			static_cast<const domain::Witness&>(*man).wake();
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
	static_cast<const domain::Witness&>(man).sleep();
	if (left)
		retell_gates(*left);
}


void ProtocolAdapter::send_auth_required(const SessionId session_id)
{
	tell_asker(session_id, v1::NOTICE_UNSPECIFIED, {}, {}, true);  // a numbered request before the token is not granted
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
		refuse(session_id, v1::NOTICE_WRITE_IN_A_ROOM);
		return;
	}
	if (const auto* abode = dynamic_cast<const domain::Abode*>(&gaze->place().source())) {
		if (&abode->host() != &man) {
			refuse(session_id, v1::NOTICE_ONLY_HOST_WRITES);
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
		// In a tie the testator wills; the novice fulfils with /done. At the gates and in
		// the upper room no one writes: they show no words.
		const auto* tie = dynamic_cast<const domain::Tie*>(&gaze->place().source());
		if (!tie) {
			refuse(session_id, v1::NOTICE_NO_WRITING_HERE);
			return;
		}
		if (tie->testator().Soul::id() != man.Soul::id()) {
			refuse(session_id, v1::NOTICE_NOVICE_FULFILS);
			return;
		}

		try {
			placed = static_cast<const domain::Testator&>(man).will(*tie, std::string{chat.body()});
		} catch (const std::exception& e) {
			refuse(session_id, e);
			return;
		}
	}

	v1::ServerEvent ack;
	*ack.mutable_receipt_ack()->mutable_word() = word_event(*placed, man.Soul::id()).word();
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

	const domain::Room* room = other.abode().room(*tie);
	if (!room)
		return;

	v1::ServerEvent event;
	auto* stirred = event.mutable_stirred();
	stirred->set_room(room->name());
	stirred->set_author_name(std::string{author.name().text()});
	send_to_vessel(other.Vessel::id(), event);
}


void ProtocolAdapter::tell_words(const SessionId session_id, const domain::Contemplation& gaze,
								 const domain::Man& listener, const std::uint32_t limit)
{
	const std::vector<std::shared_ptr<const domain::Word>> words = gaze.words();

	// How many words to send is the client's wish, not the place's concern — within what
	// a session may hold untold at once (Session::MaxPending), lest the asker be closed.
	const std::size_t told = std::min<std::size_t>(limit, MaxHistoryWords);
	const std::size_t first = words.size() > told ? words.size() - told : 0;
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
		refuse(session_id, e);
		return;
	}

	send_notice(session_id, v1::NOTICE_SUPPLICATION_SENT);

	v1::ServerEvent offer;
	offer.mutable_supplication_offer()->set_suppliant_name(std::string{self.name().text()});
	send_to_vessel(addressee_man->Vessel::id(), offer);

	// In either upper room the two now stand otherwise to each other.
	retell_upper_room(self.abode().upper_room());
	retell_upper_room(addressee_man->abode().upper_room());
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
		refuse(session_id, e);
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

	retell_upper_room(self.abode().upper_room());
	retell_upper_room(suppliant_man->abode().upper_room());
}


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


void ProtocolAdapter::handle_fulfil(const SessionId session_id, const v1::Fulfil& msg)
{
	const domain::Man& self = session_man(session_id);
	const auto& novice = static_cast<const domain::Novice&>(self);

	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(self.Soul::id());
	if (!gaze || !dynamic_cast<const domain::Tie*>(&gaze->place().source())) {
		refuse(session_id, v1::NOTICE_ENTER_TIE_ROOM);
		return;
	}

	// The number a client names is matched here, at the border, among the words he beholds.
	std::shared_ptr<const domain::Behest> behest;
	for (const std::shared_ptr<const domain::Word>& word : gaze->words()) {
		if (word->id().value() == msg.behest_id())
			behest = std::dynamic_pointer_cast<const domain::Behest>(word);
	}
	if (!behest) {
		refuse(session_id, v1::NOTICE_UNKNOWN_BEHEST);
		return;
	}

	try {
		std::optional<domain::Saying> report;
		if (!msg.report().empty())
			report.emplace(msg.report());
		std::optional<std::vector<domain::Exercise>> performed;
		if (!msg.performed().empty())
			performed = exercises_of(msg.performed());

		const std::shared_ptr<const domain::Deed> deed
			= novice.execute(*behest, std::move(report), std::move(performed));
		send_notice(session_id, v1::NOTICE_BEHEST_FULFILLED, {std::to_string(behest->id().value())});
		tell_placed(gaze->place().source(), *deed, self);
	} catch (const std::exception& e) {
		refuse(session_id, e);
	}
}


namespace {


v1::RoomAspect aspect_of(const domain::Place& place)
{
	const auto* room = dynamic_cast<const domain::Room*>(&place);
	if (!room)
		return v1::WORDS;
	switch (room->aspect()) {
	case domain::matter::Room::Aspect::Threshold:
		return v1::THRESHOLD;
	case domain::matter::Room::Aspect::Dwellers:
		return v1::DWELLERS;
	default:
		return v1::WORDS;
	}
}


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
	const domain::Gates* gates = gates_of_gaze(self);
	try {
		self.admit(*man);
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
		self.regard(*man, kind);
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	send_notice(session_id, v1::NOTICE_REGARDED, {std::string{man->name().text()}});
	tell_dwelling(self, *man);
	retell_abode(self, *man);
	retell_upper_room(self.abode().upper_room());
	retell_gates(self.abode().gates());  // his kind may let him keep them now, or no longer
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
		// The host arranges his abode here: he is shown all its rooms, each in its part.
		if (&upper_room->abode().host() == &listener)
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
				if (const domain::Room* room = abode->room(behest->tie()))
					told->set_room(room->name());
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
	const domain::Room* room = self.abode().room(msg.room());
	if (!room) {
		refuse(session_id, v1::NOTICE_NO_SUCH_ROOM);
		return;
	}

	const auto part = msg.part() == v1::OUTER ? domain::matter::Room::Part::Outer : domain::matter::Room::Part::Inner;
	try {
		self.arrange(*room, part);
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	send_notice(session_id,
				part == domain::matter::Room::Part::Outer ? v1::NOTICE_ARRANGED_OUTER : v1::NOTICE_ARRANGED_INNER,
				{msg.room()});
	send_event(session_id, arranged_rooms_event(self.abode()));
	for (const std::shared_ptr<const domain::Acquaintance>& dweller : self.abode().dwellers())
		retell_abode(self, dweller->man());
	retell_gates(self.abode().gates());  // set in another part, they are kept by other kinds
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


void ProtocolAdapter::handle_list_supplications(const SessionId session_id)
{
	const auto& self = static_cast<const domain::Testator&>(session_man(session_id));

	v1::ServerEvent event;
	auto* list = event.mutable_supplications();
	for (const std::shared_ptr<const domain::Supplication>& pending : self.supplications())
		list->add_suppliant_names(std::string{pending->suppliant().name().text()});
	send_event(session_id, event);
}


void ProtocolAdapter::handle_train(const SessionId session_id, const v1::Train& msg)
{
	const domain::Man& self = session_man(session_id);
	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(self.Soul::id());
	const auto* tie = gaze ? dynamic_cast<const domain::Tie*>(&gaze->place().source()) : nullptr;
	if (!tie) {
		refuse(session_id, v1::NOTICE_ENTER_TIE_ROOM);
		return;
	}
	if (tie->testator().Soul::id() != self.Soul::id()) {
		refuse(session_id, v1::NOTICE_ONLY_TESTATOR_TRAINS);
		return;
	}

	std::shared_ptr<const domain::Training> training;
	try {
		training = static_cast<const domain::Testator&>(self).train(
			*tie, domain::Saying{msg.title().empty() ? std::string{"Тренировка"} : msg.title()},
			exercises_of(msg.exercises()));
	} catch (const std::invalid_argument&) {
		refuse(session_id, v1::NOTICE_INVALID_TRAINING);
		return;
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	send_notice(session_id, v1::NOTICE_TRAINING_WILLED, {std::to_string(training->id().value())});
	tell_placed(*tie, *training, self);
}


std::shared_ptr<const domain::Training> ProtocolAdapter::training_in_gaze(const SessionId session_id,
																		   const domain::Man& man,
																		   const std::uint64_t behest_id)
{
	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(man.Soul::id());
	if (!gaze || !dynamic_cast<const domain::Tie*>(&gaze->place().source())) {
		refuse(session_id, v1::NOTICE_ENTER_TIE_ROOM);
		return nullptr;
	}
	for (const std::shared_ptr<const domain::Word>& word : gaze->words()) {
		if (word->id().value() == behest_id) {
			if (auto training = std::dynamic_pointer_cast<const domain::Training>(word))
				return training;
		}
	}
	refuse(session_id, v1::NOTICE_UNKNOWN_TRAINING);
	return nullptr;
}


void ProtocolAdapter::tell_tie(const domain::Tie& tie, const v1::ServerEvent& event)
{
	for (const domain::Soul& soul : world_.contemplating(tie))
		send_to_vessel(static_cast<const domain::Man&>(soul).Vessel::id(), event);
}


v1::ServerEvent ProtocolAdapter::underway_event(const domain::Tie& tie) const
{
	v1::ServerEvent event;
	auto* told = event.mutable_underway();
	if (const auto doing = static_cast<const domain::Novice&>(tie.novice()).underway()) {
		told->set_behest_id(doing->training.value());
		told->set_exercise(doing->exercise);
		told->set_approach(doing->approach);
		told->set_begun_at_ns(doing->begun.value());
		told->set_doing(true);
	}
	return event;
}


void ProtocolAdapter::handle_begin_approach(const SessionId session_id, const v1::BeginApproach& msg)
{
	const domain::Man& self = session_man(session_id);
	const std::shared_ptr<const domain::Training> training = training_in_gaze(session_id, self, msg.behest_id());
	if (!training)
		return;

	try {
		static_cast<const domain::Novice&>(self).begin(*training, msg.exercise(), msg.approach());
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}
	tell_tie(training->tie(), underway_event(training->tie()));
}


void ProtocolAdapter::handle_finish_approach(const SessionId session_id, const v1::FinishApproach& msg)
{
	const domain::Man& self = session_man(session_id);
	const std::shared_ptr<const domain::Training> training = training_in_gaze(session_id, self, msg.behest_id());
	if (!training)
		return;

	std::optional<domain::matter::Effort> effort;
	try {
		effort = static_cast<const domain::Novice&>(self).finish(*training, domain::Weight{msg.weight_grams()},
																 msg.repetitions());
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	v1::ServerEvent exerted;
	exerted.mutable_exerted()->set_behest_id(training->id().value());
	tell_effort(*effort, *exerted.mutable_exerted()->mutable_effort());
	tell_tie(training->tie(), exerted);
	tell_tie(training->tie(), underway_event(training->tie()));
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
		born = &world_.bear(midwife, *unborn);
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
			static_cast<const domain::Witness&>(child).wake();
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
		const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(man->Soul::id());
		const auto* gates = gaze ? dynamic_cast<const domain::Gates*>(&gaze->place()) : nullptr;
		if (gates && gates->keeps(*man))
			send_to_vessel(vessel, event);
	}
}


void ProtocolAdapter::handle_reject_supplication(const SessionId session_id, const v1::RejectSupplication& msg)
{
	const domain::Man* suppliant = man_named(session_id, msg.suppliant_name());
	if (!suppliant)
		return;

	const auto& self = static_cast<const domain::Testator&>(session_man(session_id));
	try {
		self.reject(*self.supplication(static_cast<const domain::Novice&>(*suppliant)));
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	send_notice(session_id, v1::NOTICE_SUPPLICATION_REJECTED, {msg.suppliant_name()});

	retell_upper_room(self.abode().upper_room());
	retell_upper_room(suppliant->abode().upper_room());
}


bool ProtocolAdapter::writes_here(const domain::Man& man) const
{
	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(man.Soul::id());
	const auto* room = gaze ? dynamic_cast<const domain::Room*>(&gaze->place()) : nullptr;
	if (!room || &room->abode() != &man.abode())
		return false;

	// One's own cell, or the room of a tie one wills in.
	if (&room->reflects() == static_cast<const domain::Place*>(&man.abode()))
		return true;
	const auto* tie = dynamic_cast<const domain::Tie*>(&room->reflects());
	return tie && tie->testator().Soul::id() == man.Soul::id();
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


const domain::Gates* ProtocolAdapter::gates_of_gaze(const domain::Man& man) const
{
	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(man.Soul::id());
	return gaze ? dynamic_cast<const domain::Gates*>(&gaze->place()) : nullptr;
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
		// The host arranges here: a tie bound or let go changes his rooms too.
		if (&man == &upper_room.abode().host())
			send_to_vessel(man.Vessel::id(), arranged_rooms_event(upper_room.abode()));
	}
}


} // namespace will
