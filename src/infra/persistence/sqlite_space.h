#pragma once

#include "horizons/space.h"
#include "sqlite_database.h"


namespace will {


class SqliteSpace final : public domain::Space {
public:
	explicit SqliteSpace(SqliteDatabase& database);

protected:
	void persist_mark(domain::id::Place id) override;

private:
	SqliteDatabase& database_;
};


} // namespace will
