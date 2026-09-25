#pragma once

#include "ports/spatiality.h"
#include "sqlite_database.h"


namespace will {


/// SQLite Spatiality — abodes and where words are placed.
class SqliteSpatiality final : public domain::Spatiality {
public:
	explicit SqliteSpatiality(SqliteDatabase& database);

	std::vector<domain::Abode> abodes() override;
	std::optional<domain::Abode> abode_of(domain::id::Soul soul) const override;
	void keep(domain::id::Abode id, domain::AbodeName name) override;
	void join_abode(domain::id::Abode abode, domain::id::Soul soul) override;
	void place(domain::id::Letter id, domain::id::Place place) override;
	std::vector<domain::Placement> placements(domain::id::Place place,
											  std::uint32_t limit) const override;

private:
	SqliteDatabase& database_;
};


} // namespace will
