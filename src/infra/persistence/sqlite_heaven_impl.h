#pragma once

#include "ports/heaven.h"
#include "sqlite_database.h"

#include <string_view>
#include <unordered_map>


namespace will {


class SqliteHeavenImpl final : public domain::Heaven {
public:
    explicit SqliteHeavenImpl(SqliteDatabase& database);

    std::optional<domain::God> find_by_device_token(std::string_view device_token) override;
    std::optional<domain::God> find_by_id(domain::GodId id) override;
    domain::God create_god(std::string_view device_token, domain::GodName name) override;

private:
    void load_all();

    SqliteDatabase& database_;
    std::unordered_map<domain::GodId, domain::God> gods_by_id_;
    std::unordered_map<domain::DeviceToken, domain::GodId> id_by_token_;
};


} // namespace will
