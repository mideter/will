#include "send_letter.h"


namespace will::domain {


SendLetter::SendLetter(LetterRepository& letters, ParticipantNotifier& notifier)
    : letters_(letters)
    , notifier_(notifier)
{}


Letter SendLetter::execute(const SendLetterInput& input)
{
    Letter saved = letters_.append(input.abode_id, input.user_id, input.body, input.created_at);
    notifier_.notify_letter(saved);
    return saved;
}


} // namespace will::domain
