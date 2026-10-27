#pragma once

#include "infra/transport/messenger.pb.h"

#include "identity/soul.h"
#include "values/exercise.h"
#include "refusals/refusal.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>


namespace will::domain {
class Abode;
class OuterAbode;
class Acquaintance;
class Place;
class Room;
class Soul;
class Word;
namespace matter {
class Effort;
class Remark;
}
} // namespace will::domain


/// How the domain is told over the wire, and brought from it: what the parts of the
/// protocol adapter share. What cannot be brought from the wire is refused
/// (std::invalid_argument).
namespace will::wire {


void tell_exercises(const std::vector<domain::Exercise>& exercises,
					google::protobuf::RepeatedPtrField<v1::Exercise>& told);


std::vector<domain::Exercise> exercises_of(const google::protobuf::RepeatedPtrField<v1::Exercise>& wire);


void tell_effort(const domain::matter::Effort& effort, v1::Effort& told);


void tell_remark(const domain::matter::Remark& remark, v1::Remark& told);


v1::ServerEvent word_event(const domain::Word& word, const domain::id::Soul listener);


std::string words_of(const v1::NoticeCode code, const std::vector<std::string>& args);


v1::NoticeCode code_of(const domain::Refusal& refusal);


std::optional<v1::RoomPart> edge_of(const domain::Place& place);


const domain::OuterAbode* abode_of(const domain::Place& place);


std::string room_of(const domain::Place& place);


std::string name_of(const domain::Room& room);


const domain::Room* room_named(const domain::Abode& abode, std::string_view name);


std::string testator_of(const domain::Place& place);


void tell_standing(v1::Turned& turned, const domain::Place& place);


void tell_room(v1::Room& told, const domain::Room& room);


v1::DwellerKind kind_of(const domain::Acquaintance& dweller);


} // namespace will::wire
