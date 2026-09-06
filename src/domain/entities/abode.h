#pragma once

#include "identity/abode.h"
#include "identity/soul.h"
#include "entities/letter.h"
#include "ports/letter_repository.h"
#include "ports/participant_notifier.h"

#include <string_view>


namespace will::domain {


/// Abode (Обитель) — place of communion in the World.
/// Holds letter memory and echoes inscribed letters to participants.
class Abode {
public:
	Abode(id::Abode id, LetterRepository& letters, ParticipantNotifier& notifier);

	id::Abode id() const noexcept { return id_; }

	/// Inscribe a letter from a soul into this abode's memory and echo it.
	Letter inscribe(id::Soul author, std::string_view body, Timestamp created_at);

private:
	id::Abode id_;
	LetterRepository& letters_;
	ParticipantNotifier& notifier_;
};


} // namespace will::domain
