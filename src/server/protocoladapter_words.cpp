/// The protocol adapter, its part of words and ties: what is written and told, the
/// supplications, behests and deeds, the trainings and their approaches.
#include "protocoladapter.h"
#include "protocoladapter_wire.h"

#include "words/behest.h"
#include "words/deed.h"
#include "words/training.h"
#include "places/abode.h"
#include "men/man.h"
#include "men/novice.h"
#include "places/obedience.h"
#include "places/shepherding.h"
#include "men/testator.h"
#include "places/tie.h"
#include "places/room.h"
#include "places/gates.h"
#include "places/helm.h"
#include "places/outer_abode.h"
#include "relations/contemplation.h"
#include "relations/supplication.h"
#include "words/letter.h"
#include "values/device_token.h"
#include "refusals/refusal.h"

#include <algorithm>
#include <cstddef>
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


namespace {


// The rooms through which the words of a place are seen: of an abode, its own; of a tie,
// the room of each side.
std::vector<std::reference_wrapper<const domain::Room>> rooms_onto(const domain::Place& place)
{
	std::vector<std::reference_wrapper<const domain::Room>> rooms;
	if (const auto* abode = dynamic_cast<const domain::Abode*>(&place)) {
		if (const domain::Room* own = abode->room(*abode))
			rooms.emplace_back(*own);
	} else if (const auto* tie = dynamic_cast<const domain::Tie*>(&place)) {
		for (const domain::Man* side : {&tie->testator(), &tie->novice()}) {
			if (const domain::Room* room = side->abode().room(*tie))
				rooms.emplace_back(*room);
		}
	}
	return rooms;
}


} // namespace


void ProtocolAdapter::handle_user_chat(const SessionId session_id, const v1::ChatMessage& chat)
{
	const auto vessel_id = registry_.vessel_id(session_id);
	if (!vessel_id)
		return;

	const domain::Man& man = world_.man(world_.vessel(*vessel_id));

	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(man);
	if (!gaze) {
		close_with_protocol_error(session_id, "Protocol error: invalid ChatMessage");
		return;
	}

	// Where he stands decides what his word becomes, or that the world refuses it.
	std::shared_ptr<const domain::Word> placed;
	try {
		placed = gaze->room().write(*gaze, domain::Saying{chat.body()});
	} catch (const domain::Refusal& refusal) {
		refuse(session_id, refusal);
		return;
	} catch (const std::exception&) {
		close_with_protocol_error(session_id, "Protocol error: invalid ChatMessage");
		return;
	}

	v1::ServerEvent ack;
	*ack.mutable_receipt_ack()->mutable_word() = word_event(*placed, man.Soul::id()).word();
	send_event(session_id, ack);

	tell_placed(gaze->room().reflects(), *placed, man);
}


void ProtocolAdapter::tell_placed(const domain::Place& place, const domain::Word& word, const domain::Man& author)
{
	// Each looking into a room onto the place sees it, if the room still admits him.
	for (const domain::Room& room : rooms_onto(place)) {
		for (const domain::Man& man : world_.contemplating(room)) {
			if (man.Soul::id() != author.Soul::id() && room.abode().shows(man, room))
				send_to_vessel(man.Vessel::id(), word_event(word, man.Soul::id()));
		}
	}

	// The other side of a tie, looking elsewhere, hears a short word of it.
	const auto* tie = dynamic_cast<const domain::Tie*>(&place);
	if (!tie)
		return;
	tell_overview(*tie);

	const domain::Man& other = tie->counterpart(author);
	const domain::Room* room = other.abode().room(*tie);
	if (!room || other.contemplates(*room))
		return;

	v1::ServerEvent event;
	auto* stirred = event.mutable_stirred();
	stirred->set_room(name_of(*room));
	stirred->set_author_name(std::string{author.name().text()});
	*stirred->mutable_word() = word_event(word, other.Soul::id()).word();
	send_to_vessel(other.Vessel::id(), event);
}


void ProtocolAdapter::tell_words(const SessionId session_id, const domain::Contemplation& gaze,
								 const domain::Man& listener, const std::uint32_t limit)
{
	const std::vector<std::shared_ptr<const domain::Word>> words = gaze.room().words(gaze);

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

	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(man);
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
	try {
		const domain::Contemplation& gaze = gaze_of(self);
		gaze.helm().supplicate(gaze, *addressee_man);
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	send_notice(session_id, v1::NOTICE_SUPPLICATION_SENT);

	v1::ServerEvent offer;
	offer.mutable_supplication_offer()->set_suppliant_name(std::string{self.name().text()});
	send_to_vessel(addressee_man->Vessel::id(), offer);

	// At either helm the two now stand otherwise to each other.
	retell_helm(self.abode().helm());
	retell_helm(addressee_man->abode().helm());
}


void ProtocolAdapter::handle_accept_supplication(const SessionId session_id,
												 const v1::AcceptSupplication& msg)
{
	const domain::Man* suppliant_man = man_named(session_id, msg.suppliant_name());
	if (!suppliant_man)
		return;

	const domain::Man& self = session_man(session_id);
	const auto& testator = self;
	const auto& novice = *suppliant_man;

	std::optional<domain::id::Place> formed;
	try {
		const domain::Contemplation& gaze = gaze_of(testator);
		formed = gaze.helm().accept(gaze, *testator.supplication(novice)).id();
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

	retell_helm(self.abode().helm());
	retell_helm(suppliant_man->abode().helm());
}


void ProtocolAdapter::handle_fulfil(const SessionId session_id, const v1::Fulfil& msg)
{
	const domain::Man& self = session_man(session_id);

	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(self);
	if (!gaze)
		return;

	// The number a client names is matched here, at the border, among the words he beholds.
	std::shared_ptr<const domain::Behest> behest;
	try {
		for (const std::shared_ptr<const domain::Word>& word : gaze->room().words(*gaze)) {
			if (word->id().value() == msg.behest_id())
				behest = std::dynamic_pointer_cast<const domain::Behest>(word);
		}
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
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
			= gaze->room().fulfil(*gaze, *behest, std::move(report), std::move(performed));
		send_notice(session_id, v1::NOTICE_BEHEST_FULFILLED, {std::to_string(behest->id().value())});
		tell_placed(deed->tie(), *deed, self);
	} catch (const std::exception& e) {
		refuse(session_id, e);
	}
}


void ProtocolAdapter::handle_list_supplications(const SessionId session_id)
{
	send_event(session_id, supplications_event(session_man(session_id)));
}


v1::ServerEvent ProtocolAdapter::supplications_event(const domain::Man& addressee) const
{
	v1::ServerEvent event;
	auto* list = event.mutable_supplications();
	for (const std::shared_ptr<const domain::Supplication>& pending : addressee.supplications())
		list->add_suppliant_names(std::string{pending->suppliant().name().text()});
	return event;
}


void ProtocolAdapter::handle_train(const SessionId session_id, const v1::Train& msg)
{
	const domain::Man& self = session_man(session_id);

	// He wills it in the room of a tie he stands in, as its host — or the world refuses.
	std::shared_ptr<const domain::Training> training;
	try {
		const domain::Contemplation& gaze = gaze_of(self);
		training = gaze.room().train(
			gaze, domain::Saying{msg.title().empty() ? std::string{"Тренировка"} : msg.title()},
			exercises_of(msg.exercises()));
	} catch (const std::invalid_argument&) {
		refuse(session_id, v1::NOTICE_INVALID_TRAINING);
		return;
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	send_notice(session_id, v1::NOTICE_TRAINING_WILLED, {std::to_string(training->id().value())});
	tell_placed(training->tie(), *training, self);
}


std::shared_ptr<const domain::Training> ProtocolAdapter::training_in_gaze(const SessionId session_id,
																		   const domain::Man& man,
																		   const std::uint64_t behest_id)
{
	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(man);
	if (!gaze)
		return nullptr;
	try {
		for (const std::shared_ptr<const domain::Word>& word : gaze->room().words(*gaze)) {
			if (word->id().value() == behest_id) {
				if (auto training = std::dynamic_pointer_cast<const domain::Training>(word))
					return training;
			}
		}
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return nullptr;
	}
	refuse(session_id, v1::NOTICE_UNKNOWN_TRAINING);
	return nullptr;
}


void ProtocolAdapter::tell_tie(const domain::Tie& tie, const v1::ServerEvent& event)
{
	for (const domain::Room& room : rooms_onto(tie)) {
		for (const domain::Man& man : world_.contemplating(room))
			send_to_vessel(man.Vessel::id(), event);
	}
}


v1::ServerEvent ProtocolAdapter::underway_event(const domain::Training& training) const
{
	v1::ServerEvent event;
	auto* told = event.mutable_underway();
	told->set_behest_id(training.id().value());
	if (const auto doing = training.underway()) {
		told->set_exercise(doing->exercise());
		told->set_approach(doing->approach());
		told->set_begun_at_ns(doing->begun().value());
		told->set_doing(true);
		if (doing->ended())
			told->set_ended_at_ns(doing->ended()->value());
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
		const domain::Contemplation& gaze = gaze_of(self);
		gaze.room().begin(gaze, *training, msg.exercise(), msg.approach());
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}
	tell_tie(training->tie(), underway_event(*training));
	tell_overview(training->tie());
}


void ProtocolAdapter::handle_end_approach(const SessionId session_id, const v1::EndApproach& msg)
{
	const domain::Man& self = session_man(session_id);
	const std::shared_ptr<const domain::Training> training = training_in_gaze(session_id, self, msg.behest_id());
	if (!training)
		return;

	try {
		const domain::Contemplation& gaze = gaze_of(self);
		gaze.room().end(gaze, *training);
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	tell_tie(training->tie(), underway_event(*training));
	tell_overview(training->tie());
}


void ProtocolAdapter::handle_finish_approach(const SessionId session_id, const v1::FinishApproach& msg)
{
	const domain::Man& self = session_man(session_id);
	const std::shared_ptr<const domain::Training> training = training_in_gaze(session_id, self, msg.behest_id());
	if (!training)
		return;

	std::optional<domain::matter::Effort> effort;
	try {
		const domain::Contemplation& gaze = gaze_of(self);
		effort = gaze.room().finish(gaze, *training, domain::Weight{msg.weight_grams()}, msg.repetitions(),
									domain::Strain{msg.strain()});
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	v1::ServerEvent exerted;
	exerted.mutable_exerted()->set_behest_id(training->id().value());
	tell_effort(*effort, *exerted.mutable_exerted()->mutable_effort());
	tell_tie(training->tie(), exerted);
	tell_tie(training->tie(), underway_event(*training));
	tell_overview(training->tie());
}


void ProtocolAdapter::handle_say_remark(const SessionId session_id, const v1::SayRemark& msg)
{
	const domain::Man& self = session_man(session_id);
	const std::shared_ptr<const domain::Training> training = training_in_gaze(session_id, self, msg.behest_id());
	if (!training)
		return;

	std::optional<domain::matter::Remark> said;
	try {
		const domain::Contemplation& gaze = gaze_of(self);
		said = gaze.room().remark(gaze, *training, msg.exercise(), domain::Saying{msg.body()});
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	v1::ServerEvent remarked;
	remarked.mutable_remarked()->set_behest_id(training->id().value());
	tell_remark(*said, *remarked.mutable_remarked()->mutable_remark());
	tell_tie(training->tie(), remarked);
}


void ProtocolAdapter::handle_reject_supplication(const SessionId session_id, const v1::RejectSupplication& msg)
{
	const domain::Man* suppliant = man_named(session_id, msg.suppliant_name());
	if (!suppliant)
		return;

	const auto& self = session_man(session_id);
	try {
		const domain::Contemplation& gaze = gaze_of(self);
		gaze.helm().reject(gaze, *self.supplication(*suppliant));
	} catch (const std::exception& e) {
		refuse(session_id, e);
		return;
	}

	send_notice(session_id, v1::NOTICE_SUPPLICATION_REJECTED, {msg.suppliant_name()});

	// As the accepted are told of their tie, the rejected are told of the refusal.
	v1::ServerEvent declined;
	auto* notice = declined.mutable_protocol_notice();
	notice->set_code(v1::NOTICE_SUPPLICATION_DECLINED);
	notice->add_args(std::string{self.name().text()});
	notice->set_message(words_of(v1::NOTICE_SUPPLICATION_DECLINED, {std::string{self.name().text()}}));
	send_to_vessel(suppliant->Vessel::id(), declined);

	retell_helm(self.abode().helm());
	retell_helm(suppliant->abode().helm());
}


} // namespace will
