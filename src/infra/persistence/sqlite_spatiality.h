#pragma once

#include "dimensions/eternity.h"
#include "dimensions/spatiality.h"
#include "sqlite_database.h"

#include <string>


namespace will {


/// SQLite Spatiality — abodes and where words are placed.
/// Place ids are pointed from Eternity's Space.
class SqliteSpatiality final : public domain::Spatiality {
public:
	/// Opens the space database at this path.
	SqliteSpatiality(std::string path, domain::Eternity& eternity);

	std::optional<domain::matter::Abode> abode(domain::id::Soul host) const override;
	domain::matter::Abode abide(domain::id::Soul host, domain::AbodeName name) override;
	domain::matter::Dweller dwell(domain::id::Place abode, domain::id::Soul soul,
								  domain::matter::Dweller::Kind kind) override;
	std::vector<domain::matter::Dweller> dwellers() const override;
	domain::matter::Room furnish(domain::id::Place abode, domain::id::Place reflects,
								 domain::matter::Room::Aspect aspect) override;
	domain::matter::Room arrange(domain::id::Place room, domain::matter::Room::Part part) override;
	std::vector<domain::matter::Room> rooms(domain::id::Place abode) const override;
	domain::matter::Tie bind(domain::id::Soul testator, domain::id::Soul novice) override;
	std::vector<domain::matter::Tie> ties() const override;
	domain::matter::Placement place(domain::id::Word id, domain::id::Place place) override;
	std::optional<domain::matter::Placement> placement(domain::id::Word id) const override;
	std::vector<domain::matter::Placement> placements(domain::id::Place place,
											  std::uint32_t limit) const override;

private:
	mutable SqliteDatabase database_;
	domain::Eternity& eternity_;
};


} // namespace will
