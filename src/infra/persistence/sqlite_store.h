#pragma once

#include "ports/eternity.h"
#include "sqlite_database.h"


namespace will {


class SqliteStore final : public domain::Eternity {
public:
    explicit SqliteStore(SqliteDatabase& database);

    std::vector<domain::Soul> load_souls() override;
    std::vector<domain::Vessel> load_vessels() override;
    domain::Man insert_soul_with_vessel(const domain::DeadVessel& dead, domain::SoulName name) override;

private:
    SqliteDatabase& database_;
};


} // namespace will
