#pragma once

#include "ports/eternity.h"
#include "sqlite_database.h"


namespace will {


class SqliteStore final : public domain::Eternity {
public:
    explicit SqliteStore(SqliteDatabase& database);

    std::vector<domain::God> load_all() override;
    domain::God insert(std::string_view device_token, domain::GodName name) override;

private:
    SqliteDatabase& database_;
};


} // namespace will
