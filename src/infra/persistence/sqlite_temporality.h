#pragma once

#include "ports/temporality.h"
#include "system_time.h"
#include "sqlite_database.h"


namespace will {


/// SQLite Temporality — mutable Eternity (souls, embodiments, time, letters).
class SqliteTemporality final : public domain::Temporality {
public:
	explicit SqliteTemporality(SqliteDatabase& database);

	domain::Time& time() override;

	domain::id::Soul enroll(domain::SoulName name) override;

	domain::Embodiment embody(domain::id::Soul soul, domain::DeviceToken token) override;
	std::vector<domain::Embodiment> embodiments() const override;

	std::vector<domain::Abode> abodes() override;
	void keep(domain::id::Abode id, domain::AbodeName name) override;
	void join_abode(domain::id::Abode abode, domain::id::Soul soul) override;
	void inscribe(domain::id::Place place, domain::id::Soul author, const domain::Word& word) const override;
	std::vector<domain::Inscription> inscriptions(domain::id::Place place, std::uint32_t limit) const override;

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
	SqliteDatabase& database_;
	SystemTime time_;
};


} // namespace will
