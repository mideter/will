#pragma once

#include "ports/temporality.h"
#include "system_time.h"
#include "sqlite_database.h"

#include <string_view>


namespace will {


/// SQLite Temporality — one body for mutable Eternity (men, time, letters).
class SqliteTemporality final : public domain::Temporality {
public:
	explicit SqliteTemporality(SqliteDatabase& database);

	domain::Time& time() override;

	std::vector<domain::Man> men() override;
	domain::Man enroll(const domain::DeviceToken& token, domain::SoulName name) override;

	std::vector<domain::Abode> abodes() override;
	void keep(domain::id::Abode id, domain::AbodeName name) override;
	void join_abode(domain::id::Abode abode, domain::id::Man man) override;
	std::vector<std::pair<domain::id::Abode, domain::id::Man>> abode_men() override;
	void fix(domain::id::Place place, domain::id::Soul author, const domain::Word& word) const override;
	std::vector<domain::Letter> letters(domain::id::Place place, std::uint32_t limit) const override;

private:
	SqliteDatabase& database_;
	SystemTime time_;
};


} // namespace will
