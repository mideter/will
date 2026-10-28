#include "protocoladapter_wire.h"
#include "refusals/world.h"
#include "refusals/training.h"
#include "refusals/tie.h"
#include "refusals/testator.h"
#include "refusals/room.h"
#include "refusals/place.h"
#include "refusals/novice.h"
#include "refusals/abode.h"
#include "places/place.h"
#include "places/tie.h"
#include "men/man.h"

#include "words/behest.h"
#include "words/deed.h"
#include "words/training.h"
#include "places/abode.h"
#include "places/room.h"
#include "places/gates.h"
#include "places/helm.h"
#include "places/outer_abode.h"
#include "relations/friend.h"
#include "relations/contemplation.h"
#include "words/letter.h"
#include "values/device_token.h"
#include "refusals/refusal.h"

#include <cstddef>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>


namespace will::wire {


void tell_exercises(const std::vector<domain::Exercise>& exercises,
					google::protobuf::RepeatedPtrField<v1::Exercise>& told)
{
	for (const domain::Exercise& exercise : exercises) {
		v1::Exercise* wire = told.Add();
		wire->set_name(exercise.name());
		wire->set_instruction(exercise.instruction());
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
		exercises.emplace_back(exercise.name(), std::move(approaches), exercise.instruction());
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
	told.set_strain(effort.strain().marks());
}


void tell_remark(const domain::matter::Remark& remark, v1::Remark& told)
{
	told.set_exercise(remark.exercise());
	told.set_body(remark.word().saying().body());
	told.set_said_at_ns(remark.dating().created_at().value());
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
			for (const domain::matter::Remark& remark : training->remarks())
				tell_remark(remark, *told->add_remarks());
		}
	} else if (const auto* deed = dynamic_cast<const domain::Deed*>(&word)) {
		told->set_kind(v1::Word::DEED);
		told->set_behest_id(deed->behest().id().value());
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
	case v1::NOTICE_SUPPLICATION_DECLINED:
		return arg(0) + " declined your supplication";
	default:
		return "refused";
	}
}


v1::NoticeCode code_of(const domain::Refusal& refusal)
{
	// The world's refusals are its beings' rules; here, at the border, each is given the
	// code the wire tells it by.
	static const std::unordered_map<std::type_index, v1::NoticeCode> codes{
		{typeid(domain::AlreadyDwells), v1::NOTICE_ALREADY_DWELLS},
		{typeid(domain::DoesNotKnock), v1::NOTICE_DOES_NOT_KNOCK},
		{typeid(domain::AlreadyKnocks), v1::NOTICE_ALREADY_KNOCKS},
		{typeid(domain::NoSuchUnborn), v1::NOTICE_NO_SUCH_UNBORN},
		{typeid(domain::DoesNotDwell), v1::NOTICE_DOES_NOT_DWELL},
		{typeid(domain::DoesNotEnterRoom), v1::NOTICE_DOES_NOT_ENTER_ROOM},
		{typeid(domain::StandInRoom), v1::NOTICE_STAND_IN_ROOM},
		{typeid(domain::StandAtHelm), v1::NOTICE_STAND_AT_HELM},
		{typeid(domain::StandAtGates), v1::NOTICE_STAND_AT_GATES},
		{typeid(domain::OnlyHostActs), v1::NOTICE_ONLY_HOST_ACTS},
		{typeid(domain::PassThroughGates), v1::NOTICE_PASS_THROUGH_GATES},
		{typeid(domain::FatherByFleshNeighbour), v1::NOTICE_FATHER_BY_FLESH_NEIGHBOUR},
		{typeid(domain::FatherBySpiritFriend), v1::NOTICE_FATHER_BY_SPIRIT_FRIEND},
		{typeid(domain::HearOnlyDweller), v1::NOTICE_HEAR_ONLY_DWELLER},
		{typeid(domain::AlreadyTied), v1::NOTICE_ALREADY_TIED},
		{typeid(domain::TieOnlyMutualDwellers), v1::NOTICE_TIE_ONLY_MUTUAL_DWELLERS},
		{typeid(domain::UnknownSupplication), v1::NOTICE_UNKNOWN_SUPPLICATION},
		{typeid(domain::WillOnlyInContemplatedTie), v1::NOTICE_WILL_ONLY_IN_CONTEMPLATED_TIE},
		{typeid(domain::OnlyTestatorWills), v1::NOTICE_ONLY_TESTATOR_WILLS},
		{typeid(domain::FulfilOnlyInContemplatedTie), v1::NOTICE_FULFIL_ONLY_IN_CONTEMPLATED_TIE},
		{typeid(domain::OnlyNoviceFulfils), v1::NOTICE_ONLY_NOVICE_FULFILS},
		{typeid(domain::AlreadyFulfilled), v1::NOTICE_ALREADY_FULFILLED},
		{typeid(domain::PlainBehestWithoutExercises), v1::NOTICE_PLAIN_BEHEST_WITHOUT_EXERCISES},
		{typeid(domain::TrainOnlyInContemplatedTie), v1::NOTICE_TRAIN_ONLY_IN_CONTEMPLATED_TIE},
		{typeid(domain::OnlyNoviceTrains), v1::NOTICE_ONLY_NOVICE_TRAINS},
		{typeid(domain::NoSuchExercise), v1::NOTICE_NO_SUCH_EXERCISE},
		{typeid(domain::NoSuchApproach), v1::NOTICE_NO_SUCH_APPROACH},
		{typeid(domain::ApproachAlreadyDone), v1::NOTICE_APPROACH_ALREADY_DONE},
		{typeid(domain::ApproachesInOrder), v1::NOTICE_APPROACHES_IN_ORDER},
		{typeid(domain::AnotherApproachUnderway), v1::NOTICE_ANOTHER_APPROACH_UNDERWAY},
		{typeid(domain::NoApproachUnderway), v1::NOTICE_NO_APPROACH_UNDERWAY},
		{typeid(domain::FinishApproachFirst), v1::NOTICE_FINISH_APPROACH_FIRST},
		{typeid(domain::TellWhatWasDone), v1::NOTICE_TELL_WHAT_WAS_DONE},
		{typeid(domain::NotOwnFather), v1::NOTICE_NOT_OWN_FATHER},
		{typeid(domain::NotDescendantAsFather), v1::NOTICE_NOT_DESCENDANT_AS_FATHER},
	};
	const auto it = codes.find(typeid(refusal));
	return it == codes.end() ? v1::NOTICE_UNSPECIFIED : it->second;
}


std::optional<v1::RoomPart> edge_of(const domain::Place& place)
{
	if (dynamic_cast<const domain::Gates*>(&place))
		return v1::OUTER;
	if (dynamic_cast<const domain::Helm*>(&place))
		return v1::INNER;
	return std::nullopt;
}


const domain::OuterAbode* abode_of(const domain::Place& place)
{
	if (const auto* abode = dynamic_cast<const domain::Abode*>(&place))
		return abode;
	if (const auto* room = dynamic_cast<const domain::Room*>(&place))
		return &room->abode();
	if (const auto* gates = dynamic_cast<const domain::Gates*>(&place))
		return &gates->abode();
	if (const auto* helm = dynamic_cast<const domain::Helm*>(&place))
		return &helm->abode();
	return nullptr;
}


std::string room_of(const domain::Place& place)
{
	const auto* room = dynamic_cast<const domain::Room*>(&place);
	return room ? name_of(*room) : std::string{};
}


std::string name_of(const domain::Room& room)
{
	// The wire knows a room by the name it is shown by: Слово of one's abode, and of a tie
	// Ведение or Послушание with the other side's name.
	if (&room.reflects() == static_cast<const domain::Place*>(&room.abode()))
		return "Слово";

	const auto& tie = dynamic_cast<const domain::Tie&>(room.reflects());
	if (tie.testator().Soul::id() == room.abode().host().Soul::id())
		return "Ведение — " + std::string{tie.novice().name().text()};
	return "Послушание — " + std::string{tie.testator().name().text()};
}


const domain::Room* room_named(const domain::Abode& abode, const std::string_view name)
{
	for (const domain::Room& room : abode.rooms()) {
		if (name_of(room) == name)
			return &room;
	}
	return nullptr;
}


void tell_standing(v1::Turned& turned, const domain::Place& place)
{
	turned.set_room(room_of(place));
	if (const auto edge = edge_of(place))
		turned.set_edge(*edge);
	turned.set_testator(testator_of(place));
}


void tell_room(v1::Room& told, const domain::Room& room)
{
	told.set_name(name_of(room));
	told.set_part(room.abode().inward(room) ? v1::INNER : v1::OUTER);
	told.set_testator(testator_of(room));
	told.set_novice(novice_of(room));
}


std::string testator_of(const domain::Place& place)
{
	// A room of a tie whose novice is its host: there the host is taught, and its testator teaches.
	const auto* room = dynamic_cast<const domain::Room*>(&place);
	const auto* tie = room ? dynamic_cast<const domain::Tie*>(&room->reflects()) : nullptr;
	if (!tie || tie->novice().Soul::id() != room->abode().host().Soul::id())
		return {};
	return std::string{tie->testator().name().text()};
}


std::string novice_of(const domain::Room& room)
{
	// A room of a tie whose testator is its host: there the host leads, and its novice is led.
	const auto* tie = dynamic_cast<const domain::Tie*>(&room.reflects());
	if (!tie || tie->testator().Soul::id() != room.abode().host().Soul::id())
		return {};
	return std::string{tie->novice().name().text()};
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
