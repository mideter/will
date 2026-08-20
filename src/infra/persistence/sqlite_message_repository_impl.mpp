module;

#include <cstdint>
#include <string_view>
#include <vector>

export module will.persistence.sqlite_message_repository;

export import will.persistence.sqlite_database;
export import will.domain.message_repository;

export namespace will {


class SqliteMessageRepositoryImpl final : public domain::MessageRepository {
public:
    explicit SqliteMessageRepositoryImpl(SqliteDatabase& database);

    domain::Message append(domain::ChatId chat, domain::UserId author, std::string_view body,
                           domain::Timestamp ts) override;

    std::vector<domain::Message> load_last(domain::ChatId chat, std::uint32_t limit) override;

private:
    SqliteDatabase& database_;
};


} // namespace will
