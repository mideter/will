#pragma once

#include "ports/eternity.h"
#include "ports/temporality.h"
#include "sqlite_database.h"


namespace will {


/// SQLite Temporality — embodiments, datings, tyings, executions in time.
/// Owns only the time database; the present is taken from Eternity.
class SqliteTemporality final : public domain::Temporality {
public:
	SqliteTemporality(SqliteDatabase& time_db, domain::Eternity& eternity);

	domain::Embodiment
	embody(domain::id::Soul soul, domain::SoulName name, domain::DeviceToken token) override;
	std::vector<domain::Embodiment> embodiments() const override;

	domain::Dating date(domain::id::Word id) override;
	std::vector<domain::Dating> datings(const std::vector<domain::id::Word>& ids) const override;

	domain::Supplication
	supplicate(const domain::Novice& suppliant, const domain::Testator& addressee) override;
	std::vector<domain::Supplication> pending_supplications(domain::id::Soul addressee) const override;
	domain::Tying accept(const domain::Supplication& ask) override;
	void reject(const domain::Supplication& ask) override;
	domain::Tying tying(domain::id::Tie id) const override;
	std::vector<domain::Tying> tyings() const override;

	void execute(domain::id::Word deed) override;
	std::vector<domain::Execution> executions(const std::vector<domain::id::Word>& ids) const override;

private:
	SqliteDatabase& time_db_;
	domain::Eternity& eternity_;
};


} // namespace will
