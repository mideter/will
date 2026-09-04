#pragma once

#include "ids/abode.h"
#include "ids/soul.h"
#include "entities/letter.h"
#include "ports/letter_repository.h"
#include "ports/participant_notifier.h"

#include <string_view>


namespace will::domain {


struct SendLetterInput {
    id::Soul soul_id;
    id::Abode abode_id = id::Abode::global();
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
