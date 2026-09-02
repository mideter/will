#pragma once

#include "ids/abode_id.h"
#include "ids/god_id.h"
#include "entities/letter.h"
#include "ports/letter_repository.h"
#include "ports/participant_notifier.h"

#include <string_view>


namespace will::domain {


struct SendLetterInput {
    GodId god_id;
    AbodeId abode_id = AbodeId::global();
    std::string_view body;
    Timestamp created_at{};
};


class SendLetter {
public:
    SendLetter(LetterRepository& letters, ParticipantNotifier& notifier);

    Letter execute(const SendLetterInput& input);

private:
    LetterRepository& letters_;
    ParticipantNotifier& notifier_;
};


} // namespace will::domain
