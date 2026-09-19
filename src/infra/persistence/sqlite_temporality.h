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
	void fix(domain::id::Place place, domain::id::Soul author, const domain::Word& word) const override;
	std::vector<domain::Letter> letters(domain::id::Place place, std::uint32_t limit) const override;

	domain::Supplication
	supplicate(const domain::Novice& suppliant, const domain::Testator& testator) override;
	std::vector<domain::Supplication> pending_supplications(domain::id::Soul testator) const override;
	domain::Obedience accept(const domain::Supplication& ask) override;
	void refuse(const domain::Supplication& ask) override;
	domain::Obedience obedience(domain::id::Obedience id) const override;
	std::vector<domain::Obedience> obediences() const override;

	domain::Deed
	bequeath(const domain::Obedience& obedience, const domain::Soul& testator,
			 const domain::Word& word) override;
	domain::Deed execute(const domain::Deed& deed) override;
	domain::Deed deed(domain::id::Deed id) const override;
	std::vector<domain::Deed> deeds(domain::id::Obedience obedience) const override;

private:
	SqliteDatabase& database_;
	SystemTime time_;
};


} // namespace will
