#pragma once

#include "ports/temporality.h"
#include "ports/time.h"
#include "sqlite_database.h"


namespace will {


/// SQLite Temporality — embodiments, datings, ties, deeds in time.
/// Owns only the time database; clock is Time from Eternity.
class SqliteTemporality final : public domain::Temporality {
public:
	SqliteTemporality(SqliteDatabase& time_db, domain::Time& time);

	domain::Embodiment
	embody(domain::id::Soul soul, domain::SoulName name, domain::DeviceToken token) override;
	std::vector<domain::Embodiment> embodiments() const override;

	void date(domain::id::Letter id, domain::Timestamp at) override;
	std::vector<domain::Dating> datings(const std::vector<domain::id::Letter>& ids) const override;

	domain::Supplication
	supplicate(const domain::Novice& suppliant, const domain::Testator& addressee) override;
	std::vector<domain::Supplication> pending_supplications(domain::id::Soul addressee) const override;
	domain::Tying accept(const domain::Supplication& ask) override;
	void reject(const domain::Supplication& ask) override;
	domain::Tying tying(domain::id::Tie id) const override;
	std::vector<domain::Tying> tyings() const override;

	domain::Deed
	will(const domain::Obedience& obedience, const domain::Soul& testator,
		 const domain::Word& word) override;
	domain::Deed execute(const domain::Deed& deed) override;
	domain::Deed deed(domain::id::Deed id) const override;
	std::vector<domain::Deed> deeds(domain::id::Tie tie) const override;

private:
	SqliteDatabase& time_db_;
	domain::Time& time_;
};


} // namespace will
