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

	domain::Letter fix(domain::id::Abode abode, domain::id::Soul author, std::string_view body) override;
	std::vector<domain::Letter> letters(domain::id::Abode abode, std::uint32_t limit) override;

private:
	SqliteDatabase& database_;
	SystemTime time_;
};


} // namespace will
