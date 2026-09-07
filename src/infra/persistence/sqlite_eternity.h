#pragma once

#include "ports/eternity.h"

#include "sqlite_database.h"


namespace will {


class SqliteEternity final : public domain::Eternity {
public:
	explicit SqliteEternity(SqliteDatabase& database);

	std::vector<domain::Man> men() override;
	domain::Man enroll(const domain::DeviceToken& token, domain::SoulName name) override;

private:
	SqliteDatabase& database_;
};


} // namespace will
