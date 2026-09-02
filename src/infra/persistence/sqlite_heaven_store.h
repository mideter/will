#pragma once

#include "ports/heaven_store.h"
#include "sqlite_database.h"


namespace will {


class SqliteHeavenStore final : public domain::HeavenStore {
public:
    explicit SqliteHeavenStore(SqliteDatabase& database);

    std::vector<domain::God> load_all() override;
    domain::God insert(std::string_view device_token, domain::GodName name) override;

private:
    SqliteDatabase& database_;
};


} // namespace will
