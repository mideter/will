#pragma once

#include "beings/space.h"
#include "sqlite_database.h"


namespace will {


/// Space with place-id mark kept in the eternity database.
class SqliteSpace final : public domain::Space {
public:
	explicit SqliteSpace(SqliteDatabase& database);

protected:
	void persist_mark(domain::id::Place id) override;

private:
	SqliteDatabase& database_;
};


} // namespace will
