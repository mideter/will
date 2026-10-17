/// The protocol adapter, its part of words and ties: what is written and told, the
/// supplications, behests and deeds, the trainings and their approaches.
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

	// Where he stands decides what his word becomes, or that the world refuses it.
	std::shared_ptr<const domain::Word> placed;
	try {
		placed = static_cast<const domain::Witness&>(man).write(domain::Saying{chat.body()});
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

	const domain::Man& other = tie->counterpart(author);
	for (const domain::Soul& beholder : world_.contemplating(place)) {
		if (beholder.id() == other.Soul::id())
			return;
	}

	const domain::Cell* cell = other.abode().cell(*tie);
	if (!cell)
		return;

	v1::ServerEvent event;
	auto* stirred = event.mutable_stirred();
	stirred->set_room(cell->name());
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


void ProtocolAdapter::handle_fulfil(const SessionId session_id, const v1::Fulfil& msg)
{
	const domain::Man& self = session_man(session_id);
	const auto& novice = static_cast<const domain::Novice&>(self);

	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(self.Soul::id());
	if (!gaze)
		return;

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
		tell_placed(deed->tie(), *deed, self);
	} catch (const std::exception& e) {
		refuse(session_id, e);
	}
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

	// He wills it in the cell of a tie he stands in, as its host — or the world refuses.
	std::shared_ptr<const domain::Training> training;
	try {
		training = static_cast<const domain::Testator&>(self).train(
			domain::Saying{msg.title().empty() ? std::string{"Тренировка"} : msg.title()},
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
	const std::shared_ptr<const domain::Contemplation> gaze = world_.contemplation(man.Soul::id());
	if (!gaze)
		return nullptr;
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


} // namespace will
