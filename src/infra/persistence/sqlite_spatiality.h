#pragma once

#include "ports/eternity.h"
#include "ports/spatiality.h"
#include "sqlite_database.h"


namespace will {


/// SQLite Spatiality — abodes and where words are placed.
/// Place ids are pointed from Eternity's Space.
class SqliteSpatiality final : public domain::Spatiality {
public:
	SqliteSpatiality(SqliteDatabase& database, domain::Eternity& eternity);

	std::optional<domain::matter::Abode> abode(domain::id::Soul host) const override;
	domain::matter::Abode abide(domain::id::Soul host, domain::AbodeName name) override;
	domain::matter::Tie bind(domain::id::Soul testator, domain::id::Soul novice) override;
	std::vector<domain::matter::Tie> ties() const override;
	void place(domain::id::Word id, domain::id::Place place) override;
	std::optional<domain::matter::Placement> placement(domain::id::Word id) const override;
	std::vector<domain::matter::Placement> placements(domain::id::Place place,
											  std::uint32_t limit) const override;

private:
	SqliteDatabase& database_;
	domain::Eternity& eternity_;
};


} // namespace will
