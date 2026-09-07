#pragma once

#include "ports/temporality.h"
#include "ports/time.h"
#include "sqlite_database.h"

#include <string_view>


namespace will {


class SqliteTemporality final : public domain::Temporality {
public:
	SqliteTemporality(SqliteDatabase& database, domain::Time& time);

	domain::Letter fix(domain::id::Abode abode, domain::id::Soul author, std::string_view body) override;

	std::vector<domain::Letter> letters(domain::id::Abode abode, std::uint32_t limit) override;

private:
	SqliteDatabase& database_;
	domain::Time& time_;
};


} // namespace will
