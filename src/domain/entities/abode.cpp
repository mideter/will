#include "abode.h"


namespace will::domain {


Abode::Abode(id::Abode id, LetterRepository& letters, ParticipantNotifier& notifier)
	: id_(id)
	, letters_(letters)
	, notifier_(notifier)
{}


Letter Abode::inscribe(id::Soul author, std::string_view body, Timestamp created_at)
{
	Letter saved = letters_.append(id_, author, body, created_at);
	notifier_.notify_letter(saved);
	return saved;
}


} // namespace will::domain
