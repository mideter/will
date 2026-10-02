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

	std::vector<domain::Abiding> abodes() override;
	domain::Abiding abide(domain::id::Soul soul, domain::AbodeName name) override;
	std::optional<domain::Abiding> abode_of(domain::id::Soul soul) const override;
	void keep(domain::id::Abode id, domain::AbodeName name) override;
	void join_abode(domain::id::Abode abode, domain::id::Soul soul) override;
	domain::Boundness bind(domain::id::Soul testator, domain::id::Soul novice) override;
	std::vector<domain::Boundness> boundnesses() const override;
	void place(domain::id::Word id, domain::id::Place place) override;
	std::optional<domain::Placement> placement(domain::id::Word id) const override;
	std::vector<domain::Placement> placements(domain::id::Place place,
											  std::uint32_t limit) const override;

private:
	SqliteDatabase& database_;
	domain::Eternity& eternity_;
};


} // namespace will
