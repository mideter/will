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


namespace will::wire {


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


void tell_effort(const domain::matter::Effort& effort, v1::Effort& told)
{
	told.set_exercise(effort.exercise());
	told.set_approach(effort.approach());
	told.set_weight_grams(effort.weight().grams());
	told.set_repetitions(effort.repetitions());
	told.set_begun_at_ns(effort.begun().value());
	told.set_finished_at_ns(effort.finished().value());
}


v1::ServerEvent word_event(const domain::Word& word, const domain::id::Soul listener)
{
	const domain::Soul& author = word.author();

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
	case v1::NOTICE_NO_SUCH_ROOM:
		return "no such room";
	case v1::NOTICE_YOU_DO_NOT_DWELL:
		return "you do not dwell in that abode";
	case v1::NOTICE_YOU_DO_NOT_ENTER:
		return "you do not enter that room";
	case v1::NOTICE_UNKNOWN_BEHEST:
		return "unknown behest";
	case v1::NOTICE_UNKNOWN_KIND:
		return "unknown dweller kind";
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


} // namespace will::wire
