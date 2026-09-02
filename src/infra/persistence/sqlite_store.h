#pragma once

#include "ports/eternity.h"
#include "sqlite_database.h"


namespace will {


class SqliteStore final : public domain::Eternity {
public:
    explicit SqliteStore(SqliteDatabase& database);

    std::vector<domain::God> load_gods() override;
    std::vector<domain::Vessel> load_vessels() override;
    std::pair<domain::God, domain::Vessel> insert_god_with_vessel(std::string_view device_token,
                                                                  domain::GodName name) override;

private:
    SqliteDatabase& database_;
};


} // namespace will
