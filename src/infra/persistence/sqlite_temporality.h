#pragma once

#include "ports/temporality.h"
#include "sqlite_database.h"

#include <string_view>


namespace will {


class SqliteTemporality final : public domain::Temporality {
public:
	explicit SqliteTemporality(SqliteDatabase& database);

	domain::Letter fix(domain::id::Abode abode, domain::id::Soul author, std::string_view body) override;

	std::vector<domain::Letter> load_last(domain::id::Abode abode, std::uint32_t limit) override;

private:
	SqliteDatabase& database_;
};


} // namespace will
