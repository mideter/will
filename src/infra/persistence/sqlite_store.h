#pragma once

#include "ports/eternity.h"
#include "sqlite_database.h"


namespace will {


class SqliteStore final : public domain::Eternity {
public:
    explicit SqliteStore(SqliteDatabase& database);

    std::vector<domain::Soul> load_souls() override;
    std::vector<domain::Vessel> load_vessels() override;
    std::pair<domain::Soul, domain::Vessel> insert_soul_with_vessel(std::string_view device_token,
                                                                  domain::SoulName name) override;

private:
    SqliteDatabase& database_;
};


} // namespace will
