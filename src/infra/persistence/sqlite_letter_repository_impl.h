#pragma once

#include "ports/letter_repository.h"
#include "sqlite_database.h"

#include <string_view>


namespace will {


class SqliteLetterRepositoryImpl final : public domain::LetterRepository {
public:
    explicit SqliteLetterRepositoryImpl(SqliteDatabase& database);

    domain::Letter append(domain::AbodeId abode, domain::GodId author, std::string_view body,
                          domain::Timestamp ts) override;

    std::vector<domain::Letter> load_last(domain::AbodeId abode, std::uint32_t limit) override;

private:
    SqliteDatabase& database_;
};


} // namespace will
