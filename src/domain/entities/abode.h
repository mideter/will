#pragma once

#include "identity/abode.h"
#include "identity/soul.h"
#include "entities/heaven.h"
#include "entities/letter.h"
#include "errors/domain_error.h"
#include "ports/letter_repository.h"
#include "ports/participant_notifier.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>


namespace will::domain {


struct RetoldLetter {
	Letter letter;
	std::string author_name;
	bool is_mine = false;
};


/// Abode (Обитель) — place of communion in the World.
/// Holds letter memory and echoes inscribed letters to participants.
class Abode {
public:
	static constexpr std::uint32_t MaxRetellLimit = 1000;

	Abode(id::Abode id, LetterRepository& letters, ParticipantNotifier& notifier, Heaven& heaven);

	id::Abode id() const noexcept { return id_; }

	/// Inscribe a letter from a soul into this abode's memory and echo it.
	Letter inscribe(id::Soul author, std::string_view body, Timestamp created_at);

	/// Retell recent letters for a soul (author names resolved through Heaven).
	std::variant<std::vector<RetoldLetter>, DomainError> retell(id::Soul soul, std::uint32_t limit) const;

private:
	id::Abode id_;
	LetterRepository& letters_;
	ParticipantNotifier& notifier_;
	Heaven& heaven_;
};


} // namespace will::domain
