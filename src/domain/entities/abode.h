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
/// Holds letter memory; echoes inscribed letters once opened through a notifier.
class Abode {
public:
	static constexpr std::uint32_t MaxRetellLimit = 1000;

	Abode(id::Abode id, LetterRepository& letters, Heaven& heaven);

	id::Abode id() const noexcept { return id_; }

	/// Open echo of inscribed letters to participants.
	void echo_through(ParticipantNotifier& notifier) noexcept;

	/// Inscribe a letter from a soul into this abode's memory and echo it.
	Letter inscribe(id::Soul author, std::string_view body, Timestamp created_at);

	/// Retell recent letters for a soul (author names resolved through Heaven).
	std::variant<std::vector<RetoldLetter>, DomainError> retell(id::Soul soul, std::uint32_t limit) const;

private:
	id::Abode id_;
	LetterRepository& letters_;
	Heaven& heaven_;
	ParticipantNotifier* notifier_ = nullptr;
};


} // namespace will::domain
