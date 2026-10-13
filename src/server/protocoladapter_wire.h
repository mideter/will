#pragma once

#include "infra/transport/messenger.pb.h"

#include "identity/soul.h"
#include "values/exercise.h"
#include "values/refusal.h"

#include <string>
#include <vector>


namespace will::domain {
class Acquaintance;
class Man;
class Place;
class Soul;
class Tie;
class Word;
namespace matter {
class Effort;
}
} // namespace will::domain


/// How the domain is told over the wire, and brought from it: what the parts of the
/// protocol adapter share.
namespace will::wire {


/// The soul whose word this is.
const domain::Soul& author_of(const domain::Word& word);


/// Exercises as the wire tells them.
void tell_exercises(const std::vector<domain::Exercise>& exercises,
					google::protobuf::RepeatedPtrField<v1::Exercise>& told);


/// Exercises as the wire brings them. Throws std::invalid_argument if any is not one.
std::vector<domain::Exercise> exercises_of(const google::protobuf::RepeatedPtrField<v1::Exercise>& wire);


/// An effort as the wire tells it.
void tell_effort(const domain::matter::Effort& effort, v1::Effort& told);


/// A word as the wire tells it to one who listens.
v1::ServerEvent word_event(const domain::Word& word, const domain::id::Soul listener);


/// The other side of a tie, seen from one of its sides.
const domain::Man& counterpart_in(const domain::Tie& tie, const domain::Man& side);


/// The server's own words for what it tells by a code, the names in it given apart.
std::string words_of(const v1::NoticeCode code, const std::vector<std::string>& args);


/// The code by which the world's reason for refusing is told.
v1::NoticeCode code_of(const domain::Refusal::Reason reason);


v1::RoomAspect aspect_of(const domain::Place& place);


v1::DwellerKind kind_of(const domain::Acquaintance& dweller);


} // namespace will::wire
