#pragma once

#include "ports/eternity.h"

#include "sqlite_database.h"


namespace will {


class SqliteStore final : public domain::Eternity {
public:
	explicit SqliteStore(SqliteDatabase& database);

	std::vector<domain::Man> recall() override;
	domain::Man insert_man(const domain::DeviceToken& token, domain::SoulName name) override;

private:
	SqliteDatabase& database_;
};


} // namespace will
