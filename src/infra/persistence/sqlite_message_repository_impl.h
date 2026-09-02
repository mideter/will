#pragma once

#include "ports/message_repository.h"
#include "sqlite_database.h"

#include <string_view>


namespace will {


class SqliteMessageRepositoryImpl final : public domain::MessageRepository {
public:
    explicit SqliteMessageRepositoryImpl(SqliteDatabase& database);

    domain::Message append(domain::AbodeId abode, domain::UserId author, std::string_view body,
                           domain::Timestamp ts) override;

    std::vector<domain::Message> load_last(domain::AbodeId abode, std::uint32_t limit) override;

private:
    SqliteDatabase& database_;
};


} // namespace will
